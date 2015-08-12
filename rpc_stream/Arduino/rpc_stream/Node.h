#ifndef ___NODE__H___
#define ___NODE__H___

#include <stdint.h>
#include <Arduino.h>
#include <Servo.h>
#include <NadaMQ.h>
#include <BaseNodeRpc.h>
#include <BaseNodeEeprom.h>
#include <BaseNodeI2c.h>
#include <BaseNodeConfig.h>
#include <BaseNodeSerialHandler.h>
#include <BaseNodeI2cHandler.h>
#include <Array.h>
#include <I2cHandler.h>
#include <SerialHandler.h>
#include <pb_validate.h>
#include <pb_eeprom.h>
#include "rpc_stream_config_validate.h"
#include "RpcStream/config_pb.h"


namespace rpc_stream {
const size_t FRAME_SIZE = (3 * sizeof(uint8_t)  // Frame boundary
                           - sizeof(uint16_t)  // UUID
                           - sizeof(uint16_t)  // Payload length
                           - sizeof(uint16_t));  // CRC

class Node;

typedef nanopb::EepromMessage<rpc_stream_Config,
                              config_validate::Validator<Node> > config_t;


class Node :
  public BaseNode,
  public BaseNodeEeprom,
  public BaseNodeI2c,
  public BaseNodeConfig<config_t>,
#ifndef DISABLE_SERIAL
  public BaseNodeSerialHandler,
#endif  // #ifndef DISABLE_SERIAL
  public BaseNodeI2cHandler<base_node_rpc::i2c_handler_t> {
public:
  typedef PacketParser<FixedPacket> parser_t;
  static const uint16_t BUFFER_SIZE = 128;  // >= longest property string
  uint16_t stream_iuid_;

  uint8_t buffer_[BUFFER_SIZE];

  Node() : BaseNode(), BaseNodeConfig<config_t>(rpc_stream_Config_fields),
           stream_iuid_(1) {}

  UInt8Array get_buffer() { return UInt8Array(sizeof(buffer_), buffer_); }
  /* This is a required method to provide a temporary buffer to the
   * `BaseNode...` classes. */

  void begin();
  void set_i2c_address(uint8_t value);  // Override to validate i2c address

  void loop() {}

  uint16_t stream_echo(UInt8Array data, uint8_t count) {
    // Echo provided data as stream packet.
    uint16_t iuid = stream_iuid_;
    for (uint8_t i = 0; i < count; i++) {
      serial_handler_.receiver_.write_f_(data, Packet::packet_type::STREAM,
                                         iuid);
    }
    stream_iuid_ += 1;
    return iuid;
  }
};


}  // namespace rpc_stream


#endif  // #ifndef ___NODE__H___
