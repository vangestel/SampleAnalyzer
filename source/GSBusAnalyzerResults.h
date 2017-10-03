#ifndef GSBUS_ANALYZER_RESULTS
#define GSBUS_ANALYZER_RESULTS

#include <AnalyzerResults.h>

class GSBusAnalyzer;
class GSBusAnalyzerSettings;

enum GSBusResultType { Channel1, Channel2, Channel3, Channel4, Channel5, Channel6, Channel7, Channel8, ErrorTooFewBits, ErrorDoesntDivideEvenly };

class GSBusAnalyzerResults : public AnalyzerResults
{
public:
	GSBusAnalyzerResults( GSBusAnalyzer* analyzer, GSBusAnalyzerSettings* settings );
	virtual ~GSBusAnalyzerResults();

	virtual void GenerateBubbleText(U64 frame_index, Channel& channel, DisplayBase display_base);
	virtual void GenerateExportFile(const char* file, DisplayBase display_base, U32 export_type_user_id);

	virtual void GenerateFrameTabularText(U64 frame_index, DisplayBase display_base);
	virtual void GeneratePacketTabularText(U64 packet_id, DisplayBase display_base);
	virtual void GenerateTransactionTabularText(U64 transaction_id, DisplayBase display_base);

protected: //functions

protected:  //vars
	GSBusAnalyzerSettings* mSettings;
	GSBusAnalyzer* mAnalyzer;
};

#endif //GSBUS_ANALYZER_RESULTS
