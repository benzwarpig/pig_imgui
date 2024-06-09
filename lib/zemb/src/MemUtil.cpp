#include "MemUtil.h"

#include <fcntl.h> /* For O_* constants */
#include <stdlib.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <unistd.h>

#include "FileUtil.h"
#include "StrUtil.h"
#include "Tracer.h"

#define USE_SEMAPHORE 0  // 使用有名信号量来进行共享内存读写互斥,默认使用文件锁

namespace zemb {
MemPool::MemPool() {}

MemPool::~MemPool() { free(m_poolStart); }

bool MemPool::init(int blockNum, int blockSize, void* memStart) {
    std::lock_guard<std::mutex> lock(m_poolMutex);
    if (blockNum <= 0 || blockSize <= 0) {
        return false;
    }
    m_blockNum = blockNum;
    m_blockSize = blockSize;
    if (memStart) {
        m_poolStart = memStart;
    } else {
        m_poolStart = calloc(m_blockNum, m_blockSize);
        if (!m_poolStart) {
            return false;
        }
    }
    for (auto i = 0; i < m_blockNum; i++) {
        auto memBlock = std::make_unique<MemBlock>();
        memBlock->isUsed = false;
        memBlock->memStart = m_poolStart + (i * m_blockSize);
        m_memBlocks.push_back(std::move(memBlock));
    }
    return true;
}

void* MemPool::getMemory(const std::string& memoryName, int memorySize) {
    std::lock_guard<std::mutex> lock(m_poolMutex);
    if (memorySize <= 0 || m_blockSize <= 0 || m_blockNum <= 0 ||
        memoryName.empty()) {
        return nullptr;
    }
    int blocks = (memorySize + m_blockSize - 1) / m_blockSize;
    int num = 0;
    for (auto i = 0; i < m_blockNum; i++) {
        if (!m_memBlocks[i]->isUsed) {
            num++;
            if (num == blocks) {
                for (auto j = 0; j < blocks; j++) {
                    m_memBlocks[i - j]->isUsed = true;
                    m_memBlocks[i - j]->memName = memoryName;
                }
                return m_memBlocks[i - blocks + 1]->memStart;
            }
        } else {
            num = 0;
        }
    }
    return nullptr;
}

bool MemPool::putMemory(const std::string& memoryName) {
    std::lock_guard<std::mutex> lock(m_poolMutex);
    if (m_blockSize <= 0 || m_blockNum <= 0) {
        return false;
    }
    for (auto i = 0; i < m_blockNum; i++) {
        if (m_memBlocks[i]->memName == memoryName && m_memBlocks[i]->isUsed) {
            m_memBlocks[i]->memName = "";
            m_memBlocks[i]->isUsed = false;
        }
    }
    return true;
}

void MemPool::showMemory() {
    for (auto i = 0; i < m_blockNum; i++) {
        TRACE_DBG("name:%s,use:%d,block:%d,addr:%08x",
                  CSTR(m_memBlocks[i]->memName), m_memBlocks[i]->isUsed, i,
                  m_memBlocks[i]->memStart);
    }
}

MemShared::MemShared() {}

MemShared::~MemShared() {
    /* shm不需要自动关闭 */
    if (m_type != SHM_MMAP) {
        close();
    }
}

bool MemShared::open(std::string name, int size, int mode,
                     MemShared::Type type) {
    if (name.empty()) {
        TRACE_ERR_CLASS("not specify shared name!");
        return false;
    }
    if (m_fd >= 0) {
        TRACE_ERR_CLASS("shared already exists!");
        return false;
    }

    int flag, mmapProt;
    switch (mode) {
        case IO_MODE_RD_ONLY: /* 只读 */
            flag = O_RDWR;
            mmapProt = PROT_READ;
            break;
        case IO_MODE_RDWR_ONLY: /* 只读写,不创建 */
            flag = O_RDWR;
            mmapProt = PROT_READ | PROT_WRITE;
            break;
        case IO_MODE_RDWR_ORNEW: /* 可读写,没有则创建 */
            flag = O_RDWR | O_CREAT;
            mmapProt = PROT_READ | PROT_WRITE;
            break;
        case IO_MODE_REWR_ORNEW: /* 重新写,没有则创建 */
            flag = O_RDWR | O_CREAT | O_TRUNC;
            mmapProt = PROT_READ | PROT_WRITE;
            break;
        default:
            TRACE_ERR_CLASS("Unsupport IO Mode: %d", mode);
            return false;
    }

    FilePath filePath(name);
    auto fileDir = filePath.dirName();
    if (!Directory::exists(fileDir)) {
        if (!Directory::createDir(fileDir, 0777, true)) {
            TRACE_ERR_CLASS("create shared dir[%s] error!", CSTR(fileDir));
            return false;
        }
    }

    /* open中的第三个参数只有在flag包含O_CREAT时有效 */
    if (type == SHM_MMAP) {
        m_fd = ::shm_open(CSTR(name), flag, 0664);
    } else {
        m_fd = ::open(CSTR(name), flag, 0664);
    }
    if (m_fd < 0) {
        TRACE_ERR_CLASS("open shared[%s] error: %s", CSTR(name), ERRSTR);
        return false;
    }
    m_type = type;
    /* 设置共享内存大小 */
    if (size > 0 && ftruncate(m_fd, size) < 0) {
        TRACE_ERR_CLASS("set shared[%s] error: %s!", CSTR(name), ERRSTR);
        close();
        return false;
    }
    /* 映射内存 */
    m_memStart = mmap(nullptr, size, mmapProt, MAP_SHARED, m_fd, 0);
    if (MAP_FAILED == m_memStart) {
        TRACE_ERR_CLASS("map shared[%s] error: %s!", CSTR(name), ERRSTR);
        m_memStart = nullptr;
        close();
        return false;
    }

#if USE_SEMAPHORE
#if 0
    auto semName = StrUtil::replaceString(name, "/", "$");
    semName = "/" + semName;
    m_procSem = std::make_shared<Semaphore>();
#else
    auto semName = name;
    m_procSem = std::make_shared<SemaphoreV>();
#endif
    TRACE_DBG_CLASS("sem name: %s", semName.data());
    if (!m_procSem->open(semName, 1)) {
        TRACE_ERR_CLASS("open semaphonre[%s] error: %s", CSTR(semName), ERRSTR);
        return false;
    }
#endif

    m_size = this->size();
    m_name = name;
    return true;
}

bool MemShared::close() {
    if (m_name.empty() || m_fd < 0) {
        return true;
    }
    if (m_memStart && munmap(m_memStart, m_size) != 0) {
        TRACE_ERR_CLASS("unmap shared[%s], error:%s", CSTR(m_name), ERRSTR);
        return false;
    }
    m_memStart = nullptr;
    if (m_type == SHM_MMAP) {
        if (shm_unlink(CSTR(m_name)) != 0) {
            TRACE_ERR_CLASS("shared[%s] unlink error: %s!", CSTR(m_name),
                            ERRSTR);
            return false;
        }
    } else {
        if ((m_fd > 0) && (::close(m_fd) != 0)) {
            TRACE_ERR_CLASS("shared[%s] close error: %s!", CSTR(m_name),
                            ERRSTR);
            return false;
        }
    }
    m_fd = -1;
#if USE_SEMAPHORE
    if (m_procSem) {
        m_procSem->close();
        m_procSem = nullptr;
    }
#endif
    return true;
}

int MemShared::putData(void* data, int size) {
#if USE_SEMAPHORE
    if (!data || size < 1 || !m_memStart || !m_procSem) {
        return -1;
    }
    m_procSem->wait();
#else
    if (!data || size < 1 || !m_memStart) {
        return -1;
    }
    flock(m_fd, LOCK_EX);  // 排他锁
#endif
#if 0
    TRACE_DBG("locked------");
    Thread::msleep(5000);
#endif
    auto len = CLIP(1, size, m_size);
    memcpy(m_memStart, data, len);
    if (m_type == MemShared::Type::FILE_MMAP) {
        if (msync(m_memStart, m_size, MS_SYNC) < 0) {
#if USE_SEMAPHORE
            m_procSem->post();
#else
            flock(m_fd, LOCK_UN);
#endif
            return -1;
        }
    }
#if USE_SEMAPHORE
    m_procSem->post();
#else
    flock(m_fd, LOCK_UN);
#endif
#if 0
    TRACE_DBG("release------");
    Thread::msleep(5000);
#endif
    return len;
}

int MemShared::getData(void* buffer, int size) {
#if USE_SEMAPHORE
    if (!buffer || size < 1 || !m_memStart || !m_procSem) {
        return -1;
    }
    m_procSem->wait();
#else
    if (!buffer || size < 1 || !m_memStart) {
        return -1;
    }
    flock(m_fd, LOCK_SH);  // 共享锁
#endif
    auto len = CLIP(1, size, m_size);
    memcpy(buffer, m_memStart, len);

#if USE_SEMAPHORE
    m_procSem->post();
#else
    flock(m_fd, LOCK_UN);
#endif

    return len;
}

int MemShared::size() {
    struct stat fdStat;
    if (m_fd < 0 || fstat(m_fd, &fdStat) < 0) {
        return 0;
    }
    return fdStat.st_size;
}

void* MemShared::data() { return m_memStart; }

}  // namespace zemb
