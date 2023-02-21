#ifndef I2C_DEVICES_INTERCHIP
#define I2C_DEVICES_INTERCHIP

struct i2c_bus {
    int sda;
    int scl;
};

//ToDoLeo this should not be exposed but is currently used in cat24c256:

#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define WRITE_BIT I2C_MASTER_WRITE  /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ    /*!< I2C master read */
#define ACK_CHECK_EN 0x1            /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0           /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                 /*!< I2C ack value */
#define NACK_VAL 0x1                /*!< I2C nack value */

esp_err_t _initInterchip(struct i2c_bus bus); //ToDoLeo this should not be exposed but is currently used in cat24c256

int i2c_receive(struct i2c_bus bus, int chip_addr, int data_addr, int len);

int i2c_send(struct i2c_bus bus, int chip_addr, int data_addr, int data, int len);

#endif