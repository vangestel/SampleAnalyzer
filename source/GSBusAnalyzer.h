#ifndef GSBUS_ANALYZER_H
#define GSBUS_ANALYZER_H

#include <Analyzer.h>
#include "GSBusAnalyzerResults.h"
#include "GSBusSimulationDataGenerator.h"

class GSBusAnalyzerSettings;
class ANALYZER_EXPORT GSBusAnalyzer : public Analyzer2
{
public:
	GSBusAnalyzer();
	virtual ~GSBusAnalyzer();

	virtual void SetupResults();
	virtual void WorkerThread();

	virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
	virtual U32 GetMinimumSampleRateHz();

	virtual const char* GetAnalyzerName() const;
	virtual bool NeedsRerun();

#pragma warning( push )
#pragma warning( disable : 4251 ) //warning C4251: 'I2sAnalyzer::<...>' : class <...> needs to have dll-interface to be used by clients of class

protected: //functions
	void AnalyzeSubFrame(U32 starting_index, U32 num_bits, U32 subframe_index);
	void AnalyzeFrame();
	void SetupForGettingFirstFrame();
	void GetFrame();
	void SetupForGettingFirstBit();
	void GetNextBit(BitState& command, BitState& status, BitState& frame, U64& sample_number);

protected:
	std::auto_ptr< GSBusAnalyzerSettings > mSettings;
	std::auto_ptr< GSBusAnalyzerResults > mResults;
	bool mSimulationInitilized;
	GSBusSimulationDataGenerator mSimulationDataGenerator;

	AnalyzerChannelData* mClock;
	AnalyzerChannelData* mFrame;
	AnalyzerChannelData* mCommand;
	AnalyzerChannelData* mStatus;

	AnalyzerResults::MarkerType mArrowMarker;

	BitState mCurrentCommand;
	BitState mCurrentStatus;
	BitState mCurrentFrame;
	U64 mCurrentSample;

	BitState mLastCommand;
	BitState mLastStatus;
	BitState mLastFrame;
	U64 mLastSample;

	std::vector<BitState> mCommandBits;
	std::vector<BitState> mStatusBits;
	std::vector<U64> mCommandValidEdges;
	std::vector<U64> mStatusValidEdges;
#pragma warning( pop )
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer();
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer(Analyzer* analyzer);

#endif //GSBUS_ANALYZER_H