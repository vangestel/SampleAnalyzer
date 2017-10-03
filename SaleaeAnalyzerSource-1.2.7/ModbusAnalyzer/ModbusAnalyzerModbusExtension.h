#ifndef MODBUS_ANALYZER_MODBUS_EXTENSION
#define MODBUS_ANALYZER_MODBUS_EXTENSION

//Function Codes
#define FUNCCODE_READ_DISCRETE_INPUTS			0x02
#define FUNCCODE_READ_COILS						0x01
#define FUNCCODE_WRITE_SINGLE_COIL				0x05
#define FUNCCODE_WRITE_MULTIPLE_COILS			0x0F
#define FUNCCODE_READ_INPUT_REGISTER			0x04
#define FUNCCODE_READ_HOLDING_REGISTERS			0x03
#define FUNCCODE_WRITE_SINGLE_REGISTER			0x06
#define FUNCCODE_WRITE_MULTIPLE_REGISTERS		0x10
#define FUNCCODE_READWRITE_MULTIPLE_REGISTERS	0x17
#define FUNCCODE_MASK_WRITE_REGISTER			0x16
#define FUNCCODE_READ_FIFO_QUEUE				0x18
#define FUNCCODE_READ_FILE_RECORD				0x14
#define FUNCCODE_WRITE_FILE_RECORD				0x15
#define	FUNCCODE_READ_EXCEPTION_STATUS			0x07
#define FUNCCODE_DIAGNOSTIC						0x08
#define	FUNCCODE_GET_COM_EVENT_COUNTER			0x0B
#define FUNCCODE_GET_COM_EVENT_LOG				0x0C
#define FUNCCODE_REPORT_SLAVE_ID				0x11
#define FUNCCODE_READ_DEVICE_ID					0x2B

#define VALUE_FRAME								0xFF

//Sub-Function Codes (used with 0x08 Diagnostics command)
#define RETURN_QUERY_DATA					0x00
#define RESTART_COMMUNICATIONS_OPTION		0x01
#define RETURN_DIAGNOSTIC_REGISTER			0x02
#define CHANGE_ASCII_INPUT_DELIM			0x03
#define FORCE_LISTEN_ONLY_MODE				0x04
	//0x05 - 0x09 RESERVED
#define CLEAR_COUNTERS_AND_DIAG_REGISTER	0x0A
#define RETURN_BUS_MESSAGE_COUNT			0x0B
#define RETURN_BUS_COMM_ERROR_COUNT			0x0C
#define RETURN_BUS_EXCEPTION_ERROR_COUNT	0x0D
#define RETURN_SLAVE_MESSAGE_COUNT			0x0E
#define RETURN_SLAVE_NO_RESPONSE_COUNT		0x0F
#define RETURN_SLAVE_NAK_COUNT				0x10
#define RETURN_SLAVE_BUSY_COUNT				0x11
#define RETURN_BUS_CHAR_OVERRUN_COUNT		0x12
	//0x13 RESERVED
#define CLEAR_OVERRUN_COUNTER_AND_FLAG		0x14
	//0x15 - 0xFFFF RESERVED


//Flags used by AnalyzerResults
#define FLAG_CHECKSUM_ERROR					0x80
#define FLAG_REQUEST_FRAME					0x40
#define FLAG_RESPONSE_FRAME					0x02
#define FLAG_EXCEPTION_FRAME				0x04
#define FLAG_DATA_FRAME						0x08
#define FLAG_END_FRAME						0x01
#define FLAG_FILE_SUBREQ					0x20

#endif //MODBUS_ANALYZER_MODBUS_EXTENSION