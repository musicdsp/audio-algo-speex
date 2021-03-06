/** @file
 * @author Edouard DUPIN 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <test-debug/debug.hpp>
#include <etk/etk.hpp>
#include <etk/uri/uri.hpp>
#include <audio/algo/speex/Resampler.hpp>
#include <audio/algo/speex/Aec.hpp>
#include <echrono/Steady.hpp>
#include <ethread/Thread.hpp>

#include <ethread/tools.hpp>



class Performance {
	private:
		echrono::Steady m_timeStart;
		echrono::Steady m_timeStop;
		echrono::Duration m_totalTimeProcessing;
		echrono::Duration m_minProcessing;
		echrono::Duration m_maxProcessing;
		int32_t m_totalIteration;
	public:
		Performance() :
		  m_totalTimeProcessing(0),
		  m_minProcessing(int64_t(99999999999999LL)),
		  m_maxProcessing(0),
		  m_totalIteration(0) {
			
		}
		void tic() {
			m_timeStart = echrono::Steady::now();
		}
		void toc() {
			m_timeStop = echrono::Steady::now();
			echrono::Duration time = m_timeStop - m_timeStart;
			m_minProcessing = etk::min(m_minProcessing, time);
			m_maxProcessing = etk::max(m_maxProcessing, time);
			m_totalTimeProcessing += time;
			m_totalIteration++;
			
		}
		
		echrono::Duration getTotalTimeProcessing() {
			return m_totalTimeProcessing;
		}
		echrono::Duration getMinProcessing() {
			return m_minProcessing;
		}
		echrono::Duration getMaxProcessing() {
			return m_maxProcessing;
		}
		int32_t getTotalIteration() {
			return m_totalIteration;
		}
		
};

float performanceResamplerStepFloat(float _sampleRateIn, float _sampleRateOut, int8_t _quality) {
	etk::Vector<float> input;
	input.resize(1024, 0);
	etk::Vector<float> output;
	output.resize(input.size()*10, 0);
	double sampleRate = _sampleRateIn;
	{
		double phase = 0;
		double baseCycle = 2.0*M_PI/sampleRate * 480.0;
		for (int32_t iii=0; iii<input.size(); iii++) {
			input[iii] = cos(phase) * 5.0;
			phase += baseCycle;
			if (phase >= 2*M_PI) {
				phase -= 2*M_PI;
			}
		}
	}
	TEST_INFO("Start Resampler performance ... " << _sampleRateIn << " -> " << _sampleRateOut << " float");
	Performance perfo;
	audio::algo::speex::Resampler algo;
	algo.init(1, _sampleRateIn, _sampleRateOut, _quality, audio::format_float);
	for (int32_t iii=0; iii<1024; ++iii) {
		perfo.tic();
		size_t sizeOut = output.size();
		algo.process(&output[0], sizeOut, &input[0], input.size());
		perfo.toc();
		ethread::sleepMilliSeconds((1));
	}
	TEST_INFO("    blockSize=" << input.size() << " sample");
	TEST_INFO("    min < avg < max =" << perfo.getMinProcessing() << " < "
	                                  << perfo.getTotalTimeProcessing().get()/perfo.getTotalIteration() << "ns < "
	                                  << perfo.getMaxProcessing());
	float avg = (float(((perfo.getTotalTimeProcessing().get()/perfo.getTotalIteration())*sampleRate)/double(input.size()))/1000000000.0)*100.0;
	TEST_INFO("    min < avg < max= " << (float((perfo.getMinProcessing().get()*sampleRate)/double(input.size()))/1000000000.0)*100.0 << "% < "
	                                  << avg << "% < "
	                                  << (float((perfo.getMaxProcessing().get()*sampleRate)/double(input.size()))/1000000000.0)*100.0 << "%");
	TEST_PRINT("float : " << _sampleRateIn << " -> " << _sampleRateOut << " quality=" << int32_t(_quality) << " : " << avg << "%");
	return avg;
}

float performanceResamplerStepI16(float _sampleRateIn, float _sampleRateOut, int8_t _quality) {
	etk::Vector<int16_t> input;
	input.resize(1024, 0);
	etk::Vector<int16_t> output;
	output.resize(input.size()*10, 0);
	double sampleRate = _sampleRateIn;
	{
		double phase = 0;
		double baseCycle = 2.0*M_PI/sampleRate * 480.0;
		for (int32_t iii=0; iii<input.size(); iii++) {
			input[iii] = cos(phase) * 30000.0;
			phase += baseCycle;
			if (phase >= 2*M_PI) {
				phase -= 2*M_PI;
			}
		}
	}
	TEST_INFO("Start Resampler performance ... " << _sampleRateIn << " -> " << _sampleRateOut << " int16_t");
	Performance perfo;
	audio::algo::speex::Resampler algo;
	algo.init(1, _sampleRateIn, _sampleRateOut, _quality, audio::format_int16);
	for (int32_t iii=0; iii<1024; ++iii) {
		perfo.tic();
		size_t sizeOut = output.size();
		algo.process(&output[0], sizeOut, &input[0], input.size());
		perfo.toc();
		ethread::sleepMilliSeconds((1));
	}
	TEST_INFO("    blockSize=" << input.size() << " sample");
	TEST_INFO("    min < avg < max =" << perfo.getMinProcessing() << " < "
	                                  << perfo.getTotalTimeProcessing().get()/perfo.getTotalIteration() << "ns < "
	                                  << perfo.getMaxProcessing());
	float avg = (float(((perfo.getTotalTimeProcessing().get()/perfo.getTotalIteration())*sampleRate)/double(input.size()))/1000000000.0)*100.0;
	TEST_INFO("    min < avg < max= " << (float((perfo.getMinProcessing().get()*sampleRate)/double(input.size()))/1000000000.0)*100.0 << "% < "
	                                  << avg << "% < "
	                                  << (float((perfo.getMaxProcessing().get()*sampleRate)/double(input.size()))/1000000000.0)*100.0 << "%");
	TEST_PRINT("int16_t : " << _sampleRateIn << " -> " << _sampleRateOut << " quality=" << int32_t(_quality) << " : " << avg << "%");
	return avg;
}

void performanceResampler() {
	for (int8_t iii=1; iii<=10; ++iii) {
		float modeFloat = performanceResamplerStepFloat(8000, 48000, iii);
		float modeI16 = performanceResamplerStepI16(8000, 48000, iii);
		modeFloat = performanceResamplerStepFloat(16000, 48000, iii);
		modeI16 = performanceResamplerStepI16(16000, 48000, iii);
		modeFloat = performanceResamplerStepFloat(32000, 48000, iii);
		modeI16 = performanceResamplerStepI16(32000, 48000, iii);
		modeFloat = performanceResamplerStepFloat(44100, 48000, iii);
		modeI16 = performanceResamplerStepI16(44100, 48000, iii);
		modeFloat = performanceResamplerStepFloat(48001, 48000, iii);
		modeI16 = performanceResamplerStepI16(48001, 48000, iii);
		modeFloat = performanceResamplerStepFloat(96000, 48000, iii);
		modeI16 = performanceResamplerStepI16(96000, 48000, iii);
		modeFloat = performanceResamplerStepFloat(48000, 96000, iii);
		modeI16 = performanceResamplerStepI16(48000, 96000, iii);
		modeFloat = performanceResamplerStepFloat(48000, 48001, iii);
		modeI16 = performanceResamplerStepI16(48000, 48001, iii);
		modeFloat = performanceResamplerStepFloat(48000, 44100, iii);
		modeI16 = performanceResamplerStepI16(48000, 44100, iii);
		modeFloat = performanceResamplerStepFloat(48000, 32000, iii);
		modeI16 = performanceResamplerStepI16(48000, 32000, iii);
		modeFloat = performanceResamplerStepFloat(48000, 16000, iii);
		modeI16 = performanceResamplerStepI16(48000, 16000, iii);
		modeFloat = performanceResamplerStepFloat(48000, 8000, iii);
		modeI16 = performanceResamplerStepI16(48000, 8000, iii);
	}
}

etk::Vector<int16_t> loadDataI16(etk::Uri _uri, int32_t _nbChannel, int32_t _selectChannel, bool _formatFileInteger16, int32_t _delaySample = 0) {
	TEST_INFO("Read : '" << _uri << "'");
	etk::Vector<int16_t> out;
	for (int32_t iii=0; iii<_delaySample; ++iii) {
		out.pushBack(0);
	}
	ememory::SharedPtr<etk::io::Interface> fileIO = etk::uri::get(_uri);
	if (fileIO->open(etk::io::OpenMode::Read) == false) {
		return out;
	}
	if (_uri.getPath().getExtention() == "wav") {
		// remove the first 44 bytes
		fileIO->seek(44, etk::io::SeekMode::Start);
	}
	if (_formatFileInteger16 == true) {
		etk::Vector<int16_t> tmpData = fileIO->readAll<int16_t>();
		for (int32_t iii=0; iii<tmpData.size(); iii+=_nbChannel) {
			out.pushBack(tmpData[iii+_selectChannel]);
		}
	} else {
		etk::Vector<float> tmpData = fileIO->readAll<float>();
		for (int32_t iii=0; iii<tmpData.size(); iii+=_nbChannel) {
			double val = double(tmpData[iii+_selectChannel])*32768.0;
			if (val >= 32767.0) {
				out.pushBack(32767);
			} else if (val <= -32768.0) {
				out.pushBack(-32768);
			} else {
				out.pushBack(int16_t(val));
			}
		}
	}
	fileIO->close();
	TEST_INFO("    " << out.size() << " samples");
	return out;
}


etk::Vector<float> loadDataFloat(etk::Uri _uri, int32_t _nbChannel, int32_t _selectChannel, bool _formatFileInteger16, int32_t _delaySample = 0) {
	TEST_INFO("Read : '" << _uri << "'");
	etk::Vector<float> out;
	for (int32_t iii=0; iii<_delaySample; ++iii) {
		out.pushBack(0.0);
	}
	ememory::SharedPtr<etk::io::Interface> fileIO = etk::uri::get(_uri);
	if (fileIO->open(etk::io::OpenMode::Read) == false) {
		return out;
	}
	if (_uri.getPath().getExtention() == "wav") {
		// remove the first 44 bytes
		fileIO->seek(44, etk::io::SeekMode::Start);
	}
	if (_formatFileInteger16 == true) {
		etk::Vector<int16_t> tmpData = fileIO->readAll<int16_t>();
		for (int32_t iii=0; iii<tmpData.size(); iii+=_nbChannel) {
			out.pushBack(double(tmpData[iii+_selectChannel])/32768.0);
		}
	} else {
		etk::Vector<float> tmpData = fileIO->readAll<float>();
		for (int32_t iii=0; iii<tmpData.size(); iii+=_nbChannel) {
			out.pushBack(tmpData[iii+_selectChannel]);
		}
	}
	fileIO->close();
	TEST_INFO("    " << out.size() << " samples");
	return out;
}

int main(int _argc, const char** _argv) {
	// the only one init for etk:
	etk::init(_argc, _argv);
	etk::Path inputName = "";
	etk::Path feedbackName = "";
	etk::Path outputName = "output.raw";
	bool performance = false;
	bool perf = false;
	int64_t sampleRateIn = 48000;
	int64_t sampleRateOut = 48000;
	int32_t nbChan = 1;
	int32_t quality = 4;
	etk::String test = "";
	bool formatFileInteger16 = true;
	int32_t inputNumberChannel = 1;
	int32_t inputSelectChannel = 0;
	int32_t inputSampleDelay = 0;
	int32_t feedbackNumberChannel = 1;
	int32_t feedbackSelectChannel = 0;
	int32_t feedbackSampleDelay = 0;
	
	for (int32_t iii=0; iii<_argc ; ++iii) {
		etk::String data = _argv[iii];
		if (etk::start_with(data,"--in=")) {
			inputName = &data[5];
		} else if (etk::start_with(data,"--out=")) {
			outputName = &data[6];
		} else if (data == "--performance") {
			performance = true;
		} else if (data == "--perf") {
			perf = true;
		} else if (etk::start_with(data,"--test=")) {
			data = &data[7];
			sampleRateIn = etk::string_to_int32_t(data);
		} else if (etk::start_with(data,"--format=")) {
			if (data == "--format=i16") {
				formatFileInteger16 = true;
			} else if (data == "--format=float") {
				formatFileInteger16 = false;
			} else {
				TEST_CRITICAL("unsuported format");
			}
		} else if (etk::start_with(data,"--in-filter=")) {
			etk::String tmpData = &data[12];
			inputNumberChannel = tmpData.size();
			for (int32_t iii = 0; iii< tmpData.size(); ++iii) {
				if (tmpData[iii] == '1') {
					inputSelectChannel = iii;
					TEST_INFO("SELECT input channel : " << inputNumberChannel+1 << " / " << tmpData.size());
					break;
				}
			}
		} else if (etk::start_with(data,"--sample-rate-in=")) {
			data = &data[17];
			sampleRateIn = etk::string_to_int32_t(data);
		// ****************************************************
		// **  RESAMPLING section
		// ****************************************************
		} else if (    test == "RESAMPLING"
		            && etk::start_with(data,"--sample-rate-out=")) {
			data = &data[18];
			sampleRateOut = etk::string_to_int32_t(data);
		} else if (    test == "RESAMPLING"
		            && etk::start_with(data,"--nb=")) {
			data = &data[5];
			nbChan = etk::string_to_int32_t(data);
		} else if (    test == "RESAMPLING"
		            && etk::start_with(data,"--quality=")) {
			data = &data[10];
			quality = etk::string_to_int32_t(data);
		// ****************************************************
		// **  AEC section
		// ****************************************************
		} else if (    test == "AEC"
		            && etk::start_with(data,"--fb-filter=")) {
			etk::String tmpData = &data[12];
			feedbackNumberChannel = tmpData.size();
			for (int32_t iii = 0; iii< tmpData.size(); ++iii) {
				if (tmpData[iii] == '1') {
					feedbackSelectChannel = iii;
					TEST_INFO("SELECT FB channel : " << feedbackSelectChannel+1 << " / " << tmpData.size());
					break;
				}
			}
		} else if (    test == "AEC"
		            && etk::start_with(data,"--fb=")) {
			feedbackName = &data[5];
		} else if (    test == "AEC"
		            && etk::start_with(data,"--fb-delay=")) {
			data = &data[11];
			feedbackSampleDelay = etk::string_to_int32_t(data);
		} else if (    test == "AEC"
		            && etk::start_with(data,"--in-delay=")) {
			data = &data[11];
			inputSampleDelay = etk::string_to_int32_t(data);
		} else if (    data == "-h"
		            || data == "--help") {
			TEST_PRINT("Help : ");
			TEST_PRINT("    ./xxx --fb=file.raw --mic=file.raw");
			TEST_PRINT("        --in=YYY.raw            input file");
			TEST_PRINT("        --in-filter=xxx         Select the channel desired in the input stream (n*0 for each channel and 1 for the selected one. ex: 4 channel, secect the third==> 0010) [default 1]");
			TEST_PRINT("        --sample-rate-in=XXXX   Input signal sample rate (default 48000)");
			TEST_PRINT("        --out=zzz.raw           output file");
			TEST_PRINT("        --format=xxx            file Format : i16/float (default i16)");
			TEST_PRINT("        --performance           Generate signal to force algo to maximum process time");
			TEST_PRINT("        --perf                  Enable performence test (little slower but real performence test)");
			TEST_PRINT("        --test=XXXX             some test availlable ...");
			TEST_PRINT("            RESAMPLING          Test resampling data 16 bit mode");
			TEST_PRINT("                --sample-rate-out=XXXX  Output signal sample rate (default 48000)");
			TEST_PRINT("                --quality=XX            Resampling quality [0..10] (default 4)");
			TEST_PRINT("                --nb=XX                 Number of channel in the file (default 1)");
			TEST_PRINT("            AEC                 Test AEC (SPEEX AEC is in 16 bits)");
			TEST_PRINT("                --fb=XXXX.raw           Input Feedback file");
			TEST_PRINT("                --fb-filter=xxx         Select the chanel desired in the input stream (same as --in-filter)");
			TEST_PRINT("                --fb-delay=xxx          dalay in sample in the signal feedback (default 0)");
			TEST_PRINT("                --in-delay=xxx          dalay in sample in the signal input (default 0)");
			TEST_PRINT("    example: ");
			TEST_PRINT("        ./XXX --test=AEC --fb=aaa_input.wav --in=aaa_input.wav --in-sample-rate=16000 --fb-filter=01 --in-filter=10 --format=i16 --in-delay=64");
			
			exit(0);
		} else {
			TEST_CRITICAL("unknow parameter : '" << data << "'");
		}
	}
	// PERFORMANCE test only ....
	if (performance == true) {
		performanceResampler();
		return 0;
	}
	if (test == "RESAMPLING") {
		TEST_INFO("Start resampling test ... ");
		if (inputName.isEmpty() == true) {
			TEST_ERROR("Can not Process missing parameters...");
			exit(-1);
		}
		TEST_INFO("Read input:");
		etk::Vector<int16_t> inputData = loadDataI16(inputName, inputNumberChannel, inputSelectChannel, formatFileInteger16, inputSampleDelay);
		TEST_INFO("    " << inputData.size() << " samples");
		// resize output :
		etk::Vector<int16_t> output;
		output.resize(inputData.size()*sampleRateOut/sampleRateIn+5000, 0);
		// process in chunk of 256 samples
		int32_t blockSize = 256*nbChan;
		
		Performance perfo;
		audio::algo::speex::Resampler algo;
		algo.init(nbChan, sampleRateIn, sampleRateOut, quality, audio::format_int16);
		int32_t lastPourcent = -1;
		size_t outputPosition = 0;
		for (int32_t iii=0; iii<inputData.size()/blockSize; ++iii) {
			if (lastPourcent != 100*iii / (inputData.size()/blockSize)) {
				lastPourcent = 100*iii / (inputData.size()/blockSize);
				TEST_INFO("Process : " << iii*blockSize << "/" << int32_t(inputData.size()/blockSize)*blockSize << " " << lastPourcent << "/100");
			} else {
				TEST_VERBOSE("Process : " << iii*blockSize << "/" << int32_t(inputData.size()/blockSize)*blockSize);
			}
			size_t availlableSize = (output.size() - outputPosition) / nbChan;
			perfo.tic();
			algo.process(&output[outputPosition], availlableSize, &inputData[iii*blockSize], blockSize);
			if (perf == true) {
				perfo.toc();
				ethread::sleepMilliSeconds(1);
			}
			outputPosition += availlableSize*nbChan;
		}
		if (perf == true) {
			TEST_INFO("Performance Result: ");
			TEST_INFO("    blockSize=" << blockSize << " sample");
			TEST_INFO("    min=" << perfo.getMinProcessing());
			TEST_INFO("    max=" << perfo.getMaxProcessing());
			TEST_INFO("    avg=" << perfo.getTotalTimeProcessing().get()/perfo.getTotalIteration() << " ns");
			
			TEST_INFO("    min=" << (float((perfo.getMinProcessing().get()*sampleRateIn)/blockSize)/1000000000.0)*100.0 << " %");
			TEST_INFO("    max=" << (float((perfo.getMaxProcessing().get()*sampleRateIn)/blockSize)/1000000000.0)*100.0 << " %");
			TEST_INFO("    avg=" << (float(((perfo.getTotalTimeProcessing().get()/perfo.getTotalIteration())*sampleRateIn)/blockSize)/1000000000.0)*100.0 << " %");
		}
		TEST_PRINT("Store in file : '" << outputName << "' size = " << output.size());
		
		
		ememory::SharedPtr<etk::io::Interface> fileIO = etk::uri::get(outputName);
		if (fileIO->open(etk::io::OpenMode::Write) == false) {
			return -1;
		}
		fileIO->writeAll<int16_t>(output);
		fileIO->close();
		///////////////////////////////////////////////////////////////////////////////////////////////////////////
	} else if (test == "AEC") {
		// process in chunk of XXX samples represent 10 ms of DATA ==> this is webRTC ...
		int32_t blockSize = 32;
		etk::Vector<int16_t> inputData = loadDataI16(inputName, inputNumberChannel, inputSelectChannel, formatFileInteger16, inputSampleDelay);
		etk::Vector<int16_t> feedbackData = loadDataI16(feedbackName, feedbackNumberChannel, feedbackSelectChannel, formatFileInteger16, feedbackSampleDelay);
		//etk::FSNodeWriteAllDataType<int16_t>("bbb_input_I16_1c.raw", inputData);
		//etk::FSNodeWriteAllDataType<int16_t>("bbb_feedback_I16_1c.raw", feedbackData);
		// resize output :
		etk::Vector<int16_t> output;
		output.resize(inputData.size(), 0);
		Performance perfo;
		{
			audio::algo::speex::Aec algo;
			algo.init(1, sampleRateIn, audio::format_int16);
			blockSize = algo.getOptimalFrameSize();
			
			int32_t lastPourcent = -1;
			for (int32_t iii=0; iii<output.size()/blockSize; ++iii) {
				if (lastPourcent != 100*iii / (output.size()/blockSize)) {
					lastPourcent = 100*iii / (output.size()/blockSize);
					TEST_INFO("Process : " << iii*blockSize << "/" << int32_t(output.size()/blockSize)*blockSize << " " << lastPourcent << "/100");
				} else {
					TEST_VERBOSE("Process : " << iii*blockSize << "/" << int32_t(output.size()/blockSize)*blockSize);
				}
				perfo.tic();
				algo.process(&output[iii*blockSize], &inputData[iii*blockSize], &feedbackData[iii*blockSize], blockSize);
				if (perf == true) {
					perfo.toc();
					ethread::sleepMilliSeconds(1);
				}
			}
		}
		TEST_PRINT("Process done");
		if (perf == true) {
			TEST_PRINT("Performance Result: ");
			TEST_INFO("    blockSize=" << blockSize << " sample");
			TEST_INFO("    min < avg < max =" << perfo.getMinProcessing().get() << "ns < "
			                                  << perfo.getTotalTimeProcessing().get()/perfo.getTotalIteration() << "ns < "
			                                  << perfo.getMaxProcessing().get() << "ns ");
			float avg = (float(((perfo.getTotalTimeProcessing().get()/perfo.getTotalIteration())*sampleRateIn)/double(blockSize))/1000000000.0)*100.0;
			TEST_INFO("    min < avg < max= " << (float((perfo.getMinProcessing().get()*sampleRateIn)/double(blockSize))/1000000000.0)*100.0 << "% < "
			                                  << avg << "% < "
			                                  << (float((perfo.getMaxProcessing().get()*sampleRateIn)/double(blockSize))/1000000000.0)*100.0 << "%");
			TEST_PRINT("float : " << sampleRateIn << " : " << avg << "%");
		}
		TEST_PRINT("Store in file : '" << outputName << "' size = " << output.size());
		ememory::SharedPtr<etk::io::Interface> fileIO = etk::uri::get(outputName);
		if (fileIO->open(etk::io::OpenMode::Write) == false) {
			return -1;
		}
		fileIO->writeAll<int16_t>(output);
		fileIO->close();
	}
	TEST_PRINT(" ***************************************");
	TEST_PRINT(" **      APPLICATION FINISHED OK      **");
	TEST_PRINT(" ***************************************");
	return 0;
}

