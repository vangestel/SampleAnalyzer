#ifndef GSBUS_ANALYZER_SETTINGS
#define GSBUS_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

enum PcmWordAlignment { LEFT_ALIGNED, RIGHT_ALIGNED };

class GSBusAnalyzerSettings : public AnalyzerSettings
{
public:
	GSBusAnalyzerSettings();
	virtual ~GSBusAnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	void UpdateInterfacesFromSettings();
	virtual void LoadSettings( const char* settings );
	virtual const char* SaveSettings();

	Channel mClockChannel;
	Channel mFrameChannel;
	Channel mCommandChannel;
	Channel mStatusChannel;

	U32 mBitsPerFrame;
	U32 mChannelsPerFrame;
	U32 mDataBitsPerChannel;
	U32 mStatusBitsPerChannel;
	U32 mParityBitsPerChannel;
	
	AnalyzerEnums::ShiftOrder mShiftOrder;
	AnalyzerEnums::EdgeDirection mDataValidEdge;
	AnalyzerEnums::Sign mSigned;

protected:
	std::auto_ptr< AnalyzerSettingInterfaceChannel > mClockChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceChannel > mFrameChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceChannel > mCommandChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceChannel > mStatusChannelInterface;

	std::auto_ptr< AnalyzerSettingInterfaceNumberList > mBitsPerFrameInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList > mChannelsPerFrameInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList > mDataBitsPerChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList > mStatusBitsPerChannelInterface;

	std::auto_ptr< AnalyzerSettingInterfaceNumberList > mShiftOrderInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList > mDataValidEdgeInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList > mSignedInterface;
};

#endif //GSBUS_ANALYZER_SETTINGS
