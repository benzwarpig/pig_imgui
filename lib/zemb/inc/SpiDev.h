#ifndef __ZEMB_SPI_DEV_H__
#define __ZEMB_SPI_DEV_H__

#include <string>

#include "BaseType.h"
namespace zemb {
#if 0 /* SPI mode定义(linux/spi/spidev.h) */
/* User space versions of kernel symbols for SPI clocking modes,
 * matching <linux/spi/spi.h>
 */
#define SPI_CPHA 0x01
#define SPI_CPOL 0x02

#define SPI_MODE_0 (0 | 0)
#define SPI_MODE_1 (0 | SPI_CPHA)
#define SPI_MODE_2 (SPI_CPOL | 0)
#define SPI_MODE_3 (SPI_CPOL | SPI_CPHA)

#define SPI_CS_HIGH 0x04
#define SPI_LSB_FIRST 0x08
#define SPI_3WIRE 0x10
#define SPI_LOOP 0x20
#define SPI_NO_CS 0x40
#define SPI_READY 0x80
#define SPI_TX_DUAL 0x100
#define SPI_TX_QUAD 0x200
#define SPI_RX_DUAL 0x400
#define SPI_RX_QUAD 0x800
#endif

class SpiDev {
    DECL_CLASSNAME(SpiDev)

public:
    SpiDev();
    virtual ~SpiDev();
    /**
     * @brief 打开SPI设备
     * @param name 设备名称
     * @param mode SPI模式
     * @param speed 时钟速率(uint:HZ)
     * @param usdelay 两个transfer之间的延时(uint:us)
     * @param bits 字长(bits per word)
     * @return true
     * @return false
     */
    bool open(const std::string& name, uint32 mode, uint32 speed,
              uint16 usdelay = 0, uint8 bits = 8);
    /**
     * @brief 传输数据
     * @param txbuf 发送缓存
     * @param txlen
     * @param rxbuf 接收缓存
     * @param rxlen
     * @return true
     * @return false
     */
    bool transfer(uint8* txbuf, uint32 txlen, uint8* rxbuf, uint32 rxlen);
    void close();

private:
    int m_fd{-1};
    uint32 m_mode{0};
    uint32 m_speed{500000};
    uint16 m_usdelay{0};
    uint8 m_bits{8};
};
}  // namespace zemb
#endif
