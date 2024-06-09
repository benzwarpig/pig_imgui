#include "SpiDev.h"

#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "StrUtil.h"
#include "Tracer.h"

namespace zemb {

SpiDev::SpiDev() {}

SpiDev::~SpiDev() { close(); }

bool SpiDev::open(const std::string& name, uint32 mode, uint32 speed,
                  uint16 usdelay, uint8 bits) {
    m_fd = ::open(CSTR(name), O_RDWR);
    if (m_fd < 0) {
        TRACE_ERR_CLASS("open %s error!", CSTR(name));
        return false;
    }

    if (mode & SPI_LOOP) {
        if (mode & SPI_TX_DUAL) {
            mode |= SPI_RX_DUAL;
        }
        if (mode & SPI_TX_QUAD) {
            mode |= SPI_RX_QUAD;
        }
    }
    if (ioctl(m_fd, SPI_IOC_WR_MODE, &mode) < 0) {
        TRACE_ERR_CLASS("set SPI mode error, mode: 0x%08x", mode);
        return false;
    }

    if (ioctl(m_fd, SPI_IOC_RD_MODE, &m_mode) < 0) {
        TRACE_ERR_CLASS("get SPI mode error: %s", ERRSTR);
        return false;
    }

    if (ioctl(m_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
        TRACE_ERR_CLASS("set SPI bits error, bits: %d", bits);
        return false;
    }

    if (ioctl(m_fd, SPI_IOC_RD_BITS_PER_WORD, &m_bits) < 0) {
        TRACE_ERR_CLASS("get SPI bits error: %s", ERRSTR);
        return false;
    }

    if (ioctl(m_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        TRACE_ERR_CLASS("set SPI speed error, speed: %d", speed);
        return false;
    }

    if (ioctl(m_fd, SPI_IOC_RD_MAX_SPEED_HZ, &m_speed)) {
        TRACE_ERR_CLASS("get SPI speed error: %s", ERRSTR);
        return false;
    }
    m_usdelay = usdelay;
    TRACE_DBG_CLASS("spidev: %s, mode: 0x%08x, bits: %d, speed: %d", CSTR(name),
                    m_mode, m_bits, m_speed);
    return true;
}

bool SpiDev::transfer(uint8* txbuf, uint32 txlen, uint8* rxbuf, uint32 rxlen) {
    if (!txbuf || txlen == 0) {
        TRACE_ERR_CLASS("txbuf cannot be null!");
        return false;
    }
    uint32 len = txlen;
    uint8* buffer = nullptr;
    if (rxbuf && rxlen > 0) { /* 需要接收数据 */
        len += rxlen;
        buffer = new uint8[len];
        memset(buffer, 0, len);
    }
    struct spi_ioc_transfer sit {
        0
    };
    sit.tx_buf = (unsigned long)txbuf;  /* NOLINT */
    sit.rx_buf = (unsigned long)buffer; /* NOLINT */
    sit.len = len;
    sit.delay_usecs = m_usdelay;
    sit.speed_hz = m_speed;
    sit.bits_per_word = m_bits;

    if (m_mode & SPI_TX_QUAD) {
        sit.tx_nbits = 4;
    } else if (m_mode & SPI_TX_DUAL) {
        sit.tx_nbits = 2;
    }
    if (m_mode & SPI_RX_QUAD) {
        sit.rx_nbits = 4;
    } else if (m_mode & SPI_RX_DUAL) {
        sit.rx_nbits = 2;
    }
    if (!(m_mode & SPI_LOOP)) {
        if (m_mode & (SPI_TX_QUAD | SPI_TX_DUAL)) {
            sit.rx_buf = 0;
        } else if (m_mode & (SPI_RX_QUAD | SPI_RX_DUAL)) {
            sit.tx_buf = 0;
        }
    }

    if (ioctl(m_fd, SPI_IOC_MESSAGE(1), &sit) < 1) {
        TRACE_ERR_CLASS("SPI_IOC_MESSAGE error: %s", ERRSTR);
        if (buffer) {
            free(buffer);
        }
        return false;
    }
    if (buffer) {
        memcpy(rxbuf, buffer + txlen, rxlen);
        free(buffer);
    }
    return true;
}

void SpiDev::close() {
    if (m_fd > 0) {
        ::close(m_fd);
    }
}
}  // namespace zemb
