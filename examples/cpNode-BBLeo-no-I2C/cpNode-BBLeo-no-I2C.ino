/**
 * cpNode - Control Point CMRI Node
 * =================================
 * This sketch supports a minimal cpNode with no IOX expanders
 * 
 * The heavy lifting is done behind the scenes with the following class libraries:
 *     cpCMRI:  Implements all the protocol handling fiddly bits to work with CMRInet
 *              includes
 *                    CMRI_Packet: The details of a CMRInet packet structure.
 *                    cpIOMap: Abstracts the reading and writing of bits to ports/pins and devices
 *     I2Cexpander:  abstracts the initialization, reading and writing details of 8- and 16-bit I2C expanders
 */

#include <cpCMRI.h>
#include <I2Cexpander.h>
#define DEBUG 1 // enable Serial printing of debug messages
// #define DEBUG 0
#define TRACE() if (DEBUG)

//==============================================
//====   BEGIN CONFIGURATION PARAMETERS     ====
//==============================================

#define CMRINET_NODE_ID        1  // can be [0..64]  change this - must be unique for each node...
#define CMRINET_SPEED      19200  // make sure this matches the speed set in JMRI

cpIOMap node_configuration[] = {
    // device                 pin or                              '1'/'0' = initialized output ' ' = dontcare
    // type                    addr  I/O               initilize   '+'    = input pullup, ' ' = input HiZ
  { I2Cexpander::BUILTIN,      11,   "I",                "+"},
  { I2Cexpander::BUILTIN,      10,   "I",                "+"},
  { I2Cexpander::BUILTIN,       9,   "I",                "+"},
  { I2Cexpander::BUILTIN,       8,   "i",                "+"},
  { I2Cexpander::BUILTIN,       7,   "i",                " "},
  { I2Cexpander::BUILTIN,       6,   "1",                " "},
  { I2Cexpander::BUILTIN,       5,   "I",                " "},
  { I2Cexpander::BUILTIN,       4,   "I",                " "},
  { I2Cexpander::BUILTIN,      13,   "O",                "1"},
  { I2Cexpander::BUILTIN,      12,   "O",                "0"},
  { I2Cexpander::BUILTIN,      A0,   "O",                "0"},
  { I2Cexpander::BUILTIN,      A1,   "O",                "0"},
  { I2Cexpander::BUILTIN,      A2,   "o",                "1"},
  { I2Cexpander::BUILTIN,      A3,   "o",                "1"},
  { I2Cexpander::BUILTIN,      A4,   "o",                "0"},
  { I2Cexpander::BUILTIN,      A5,   "o",                "1"},
  _END_OF_IOMAP_LIST_
};

//==============================================
//====    END CONFIGURATION PARAMETERS      ====
//==============================================

CMRI_Node *node;

/**
 * These routines are called automatically when the protocol_handler() routine gets either a 
 * POLL or a TX packet.
 * 
 * POLL calls out to this routine to gather input values and put them into the provided packet 
 */
void gatherInputs(CMRI_Packet &p) {
      cpIOMap::collectIOMapInputs(node_configuration, p.content());
      TRACE() { Serial.print("POLL:==>\nRX: <== "); Serial.println(CMRI_Node::packetToString(p)); }
}

/**
 * When a TX packet is received, this routine needs to distribute the output bits to the 
 * pins and devices that need them.
 */
void distributeOutputs(CMRI_Packet &p) {
      // TRACE() { Serial.print("TX: ==> "); Serial.println(CMRI_Node::packetToString(p));  }
      cpIOMap::distributeIOMapOutputs(node_configuration, p.content());
}

void errorHandler(CMRI_Packet &p) {
      TRACE() { Serial.print("ERROR: ==> "); Serial.println(CMRI_Node::packetToString(p));  }
}

void setup() {
    TRACE() {
        Serial.begin(115200);
        while (!Serial) {
          ; // wait for serial port to connect. Needed for native USB
        }
        Serial.print("CMRI Node ");
        Serial.print("Address ");
        Serial.print(CMRINET_NODE_ID, DEC);
        Serial.println(" - cpNode + 8x IOX-16 example");
    }
    
    Serial1.begin(CMRINET_SPEED, SERIAL_8N2);

    cpIOMap::setupIOMap(node_configuration);

    node = new CMRI_Node(CMRINET_NODE_ID, Serial1);
    node->set_num_input_bits(cpIOMap::countIOMapInputs(node_configuration));  // how many Input bits?
    node->set_num_output_bits(cpIOMap::countIOMapOutputs(node_configuration)); // how many output bits?
    node->setInputHandler(gatherInputs);
    node->setOutputHandler(distributeOutputs);
    node->setErrorHandler(errorHandler);

    TRACE() {
        Serial.println("Configured for:");
        Serial.print("    "); Serial.print(CMRINET_SPEED);  Serial.println(" Baud");
        Serial.print("    "); Serial.print(node->get_num_input_bits());  Serial.println(" Inputs");
        Serial.print("    "); Serial.print(node->get_num_output_bits()); Serial.println(" Outputs");
    }
}

void loop() {
    node->protocol_handler();
}
