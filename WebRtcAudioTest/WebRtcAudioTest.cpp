// WebRtcAudioTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

#include "../WebRtcMoudle/signal_processing_library.h"
#include "../WebRtcMoudle/noise_suppression_x.h"
#include "../WebRtcMoudle/noise_suppression.h"
#include "../WebRtcMoudle/gain_control.h"

/* 重新写的三种采样率 */
void NoiseSuppression(char* szFileIn, char* szFileOut, int nSample, int nMode)
{
	int nRet = 0;
	NsHandle* pNS_inst = NULL;

	FILE* fpIn = NULL;
	FILE* fpOut = NULL;

	char* pInBuffer = NULL;
	char* pOutBuffer = NULL;

	do
	{
		int i = 0;
		int nFileSize = 0;
		int nTime = 0;
		if (0 != WebRtcNs_Create(&pNS_inst))
		{
			printf("Noise_Suppression WebRtcNs_Create err! \n");
			break;
		}

		if (0 != WebRtcNs_Init(pNS_inst, nSample))
		{
			printf("Noise_Suppression WebRtcNs_Init err! \n");
			break;
		}

		if (0 != WebRtcNs_set_policy(pNS_inst, nMode))
		{
			printf("Noise_Suppression WebRtcNs_set_policy err! \n");
			break;
		}

		fpIn = fopen(szFileIn, "rb");
		if (NULL == fpIn)
		{
			printf("open src file err \n");
			break;
		}
		fseek(fpIn, 0, SEEK_END);
		nFileSize = ftell(fpIn);
		fseek(fpIn, 0, SEEK_SET);

		pInBuffer = (char*)malloc(nFileSize);
		memset(pInBuffer, 0, nFileSize);
		fread(pInBuffer, sizeof(char), nFileSize, fpIn);

		pOutBuffer = (char*)malloc(nFileSize);
		memset(pOutBuffer, 0, nFileSize);

		int  filter_state1[6], filter_state12[6];
		int  Synthesis_state1[6], Synthesis_state12[6];

		memset(filter_state1, 0, sizeof(filter_state1));
		memset(filter_state12, 0, sizeof(filter_state12));
		memset(Synthesis_state1, 0, sizeof(Synthesis_state1));
		memset(Synthesis_state12, 0, sizeof(Synthesis_state12));

		nTime = GetTickCount();
		for (i = 0; i < nFileSize; i += sizeof(short) * nSample / 100)
		{
			if (nSample == 8000)
			{
				if (nFileSize - i >= 160) {
					short shBufferIn[80] = { 0 };
					short shBufferOut[80] = { 0 };
					memcpy(shBufferIn, (char*)(pInBuffer + i), 80 * sizeof(short));
					//将需要降噪的数据作为低频数据传入对应接口，高频数据为空，同时需要注意返回数据也是分高频和低频，此处低频数据为降噪结果
					if (0 != WebRtcNs_Process(pNS_inst, shBufferIn, NULL, shBufferOut, NULL)) {
						printf("Noise_Suppression WebRtcNs_Process err! \n");
					}
					memcpy(pOutBuffer + i, shBufferOut, 80 * sizeof(short));
				}
			}
			else if (nSample == 16000)
			{
				if (nFileSize - i >= 320) {
					short shBufferIn[160] = { 0 };
					short shBufferOut[160] = { 0 };
					memcpy(shBufferIn, (char*)(pInBuffer + i), 160 * sizeof(short));
					//将需要降噪的数据作为低频数据传入对应接口，高频数据为空，同时需要注意返回数据也是分高频和低频，此处低频数据为降噪结果
					if (0 != WebRtcNs_Process(pNS_inst, shBufferIn, NULL, shBufferOut, NULL)) {
						printf("Noise_Suppression WebRtcNs_Process err! \n");
					}
					memcpy(pOutBuffer + i, shBufferOut, 160 * sizeof(short));
				}
			}
			else if (nSample == 32000)
			{
				if (nFileSize - i >= 640)
				{
					short shBufferIn[320] = { 0 };

					short shInL[160], shInH[160];
					short shOutL[160] = { 0 }, shOutH[160] = { 0 };

					memcpy(shBufferIn, (char*)(pInBuffer + i), 320 * sizeof(short));
					//首先需要使用滤波函数将音频数据分高低频，以高频和低频的方式传入降噪函数内部
					WebRtcSpl_AnalysisQMF(shBufferIn, 320, shInL, shInH, filter_state1, filter_state12);

					//将需要降噪的数据以高频和低频传入对应接口，同时需要注意返回数据也是分高频和低频
					if (0 == WebRtcNs_Process(pNS_inst, shInL, shInH, shOutL, shOutH))
					{
						short shBufferOut[320];
						//如果降噪成功，则根据降噪后高频和低频数据传入滤波接口，然后用将返回的数据写入文件
						WebRtcSpl_SynthesisQMF(shOutL, shOutH, 160, shBufferOut, Synthesis_state1, Synthesis_state12);
						memcpy(pOutBuffer + i, shBufferOut, 320 * sizeof(short));
					}
				}
			}
		}

		nTime = GetTickCount() - nTime;
		printf("n_s user time=%dms\n", nTime);
		fpOut = fopen(szFileOut, "wb");
		if (NULL == fpOut)
		{
			printf("open out file err! \n");
			break;
		}
		fwrite(pOutBuffer, sizeof(char), nFileSize, fpOut);
	} while (0);

	WebRtcNs_Free(pNS_inst);
	fclose(fpIn);
	fclose(fpOut);
	free(pInBuffer);
	free(pOutBuffer);
}

void NoiseSuppression32(char* szFileIn, char* szFileOut, int nSample, int nMode)
{
	int nRet = 0;
	NsHandle* pNS_inst = NULL;

	FILE* fpIn = NULL;
	FILE* fpOut = NULL;

	char* pInBuffer = NULL;
	char* pOutBuffer = NULL;

	do
	{
		int i = 0;
		int nFileSize = 0;
		int nTime = 0;
		if (0 != WebRtcNs_Create(&pNS_inst))
		{
			printf("Noise_Suppression WebRtcNs_Create err! \n");
			break;
		}

		if (0 != WebRtcNs_Init(pNS_inst, nSample))
		{
			printf("Noise_Suppression WebRtcNs_Init err! \n");
			break;
		}

		if (0 != WebRtcNs_set_policy(pNS_inst, nMode))
		{
			printf("Noise_Suppression WebRtcNs_set_policy err! \n");
			break;
		}

		fpIn = fopen(szFileIn, "rb");
		if (NULL == fpIn)
		{
			printf("open src file err \n");
			break;
		}
		fseek(fpIn, 0, SEEK_END);
		nFileSize = ftell(fpIn);
		fseek(fpIn, 0, SEEK_SET);

		pInBuffer = (char*)malloc(nFileSize);
		memset(pInBuffer, 0, nFileSize);
		fread(pInBuffer, sizeof(char), nFileSize, fpIn);

		pOutBuffer = (char*)malloc(nFileSize);
		memset(pOutBuffer, 0, nFileSize);

		int  filter_state1[6], filter_state12[6];
		int  Synthesis_state1[6], Synthesis_state12[6];

		memset(filter_state1, 0, sizeof(filter_state1));
		memset(filter_state12, 0, sizeof(filter_state12));
		memset(Synthesis_state1, 0, sizeof(Synthesis_state1));
		memset(Synthesis_state12, 0, sizeof(Synthesis_state12));

		nTime = GetTickCount();
		for (i = 0; i < nFileSize; i += 640)
		{
			if (nFileSize - i >= 640)
			{
				short shBufferIn[320] = { 0 };

				short shInL[160], shInH[160];
				short shOutL[160] = { 0 }, shOutH[160] = { 0 };

				memcpy(shBufferIn, (char*)(pInBuffer + i), 320 * sizeof(short));
				//首先需要使用滤波函数将音频数据分高低频，以高频和低频的方式传入降噪函数内部
				WebRtcSpl_AnalysisQMF(shBufferIn, 320, shInL, shInH, filter_state1, filter_state12);

				//将需要降噪的数据以高频和低频传入对应接口，同时需要注意返回数据也是分高频和低频
				if (0 == WebRtcNs_Process(pNS_inst, shInL, shInH, shOutL, shOutH))
				{
					short shBufferOut[320];
					//如果降噪成功，则根据降噪后高频和低频数据传入滤波接口，然后用将返回的数据写入文件
					WebRtcSpl_SynthesisQMF(shOutL, shOutH, 160, shBufferOut, Synthesis_state1, Synthesis_state12);
					memcpy(pOutBuffer + i, shBufferOut, 320 * sizeof(short));
				}
			}
		}

		nTime = GetTickCount() - nTime;
		printf("n_s user time=%dms\n", nTime);
		fpOut = fopen(szFileOut, "wb");
		if (NULL == fpOut)
		{
			printf("open out file err! \n");
			break;
		}
		fwrite(pOutBuffer, sizeof(char), nFileSize, fpOut);
	} while (0);

	WebRtcNs_Free(pNS_inst);
	fclose(fpIn);
	fclose(fpOut);
	free(pInBuffer);
	free(pOutBuffer);
}

/* 原始的32k采样率 */
void NoiseSuppressionX32(char* szFileIn, char* szFileOut, int nSample, int nMode)
{
	int nRet = 0;
	NsxHandle* pNS_inst = NULL;

	FILE* fpIn = NULL;
	FILE* fpOut = NULL;

	char* pInBuffer = NULL;
	char* pOutBuffer = NULL;

	do
	{
		int i = 0;
		int nFileSize = 0;
		int nTime = 0;
		if (0 != WebRtcNsx_Create(&pNS_inst))
		{
			printf("Noise_Suppression WebRtcNs_Create err! \n");
			break;
		}

		if (0 != WebRtcNsx_Init(pNS_inst, nSample))
		{
			printf("Noise_Suppression WebRtcNs_Init err! \n");
			break;
		}

		if (0 != WebRtcNsx_set_policy(pNS_inst, nMode))
		{
			printf("Noise_Suppression WebRtcNs_set_policy err! \n");
			break;
		}

		fpIn = fopen(szFileIn, "rb");
		if (NULL == fpIn)
		{
			printf("open src file err \n");
			break;
		}
		fseek(fpIn, 0, SEEK_END);
		nFileSize = ftell(fpIn);
		fseek(fpIn, 0, SEEK_SET);

		pInBuffer = (char*)malloc(nFileSize);
		memset(pInBuffer, 0, nFileSize);
		fread(pInBuffer, sizeof(char), nFileSize, fpIn);

		pOutBuffer = (char*)malloc(nFileSize);
		memset(pOutBuffer, 0, nFileSize);

		int  filter_state1[6], filter_state12[6];
		int  Synthesis_state1[6], Synthesis_state12[6];

		memset(filter_state1, 0, sizeof(filter_state1));
		memset(filter_state12, 0, sizeof(filter_state12));
		memset(Synthesis_state1, 0, sizeof(Synthesis_state1));
		memset(Synthesis_state12, 0, sizeof(Synthesis_state12));

		nTime = GetTickCount();
		for (i = 0; i < nFileSize; i += 640)
		{
			if (nFileSize - i >= 640)
			{
				short shBufferIn[320] = { 0 };

				short shInL[160], shInH[160];
				short shOutL[160] = { 0 }, shOutH[160] = { 0 };

				memcpy(shBufferIn, (char*)(pInBuffer + i), 320 * sizeof(short));
				//首先需要使用滤波函数将音频数据分高低频，以高频和低频的方式传入降噪函数内部
				WebRtcSpl_AnalysisQMF(shBufferIn, 320, shInL, shInH, filter_state1, filter_state12);

				//将需要降噪的数据以高频和低频传入对应接口，同时需要注意返回数据也是分高频和低频
				if (0 == WebRtcNsx_Process(pNS_inst, shInL, shInH, shOutL, shOutH))
				{
					short shBufferOut[320];
					//如果降噪成功，则根据降噪后高频和低频数据传入滤波接口，然后用将返回的数据写入文件
					WebRtcSpl_SynthesisQMF(shOutL, shOutH, 160, shBufferOut, Synthesis_state1, Synthesis_state12);
					memcpy(pOutBuffer + i, shBufferOut, 320 * sizeof(short));
				}
			}
		}

		nTime = GetTickCount() - nTime;
		printf("n_s user time=%dms\n", nTime);
		fpOut = fopen(szFileOut, "wb");
		if (NULL == fpOut)
		{
			printf("open out file err! \n");
			break;
		}
		fwrite(pOutBuffer, sizeof(char), nFileSize, fpOut);
	} while (0);

	WebRtcNsx_Free(pNS_inst);
	fclose(fpIn);
	fclose(fpOut);
	free(pInBuffer);
	free(pOutBuffer);
}

void WebRtcAgcTest(char* filename, char* outfilename, int fs)
{
	FILE* infp = NULL;
	FILE* outfp = NULL;

	short* pData = NULL;
	short* pOutData = NULL;
	void* agcHandle = NULL;
	
	do
	{
		WebRtcAgc_Create(&agcHandle);

		int minLevel = 0;
		int maxLevel = 255;    // minLevel和maxLevel分别是音量的最小值和最大值
		/*
		 * kAgcModeUnchanged,
		 * kAgcModeAdaptiveAnalog,  // 自适应模拟模式
		 * kAgcModeAdaptiveDigital, // 自适应数字增益模式
		 * kAgcModeFixedDigital  // 固定数字增益模式
		 */
		int agcMode = kAgcModeFixedDigital;    // 4种模式第一个模式是什么都不改变，但是会作削顶保护，然后是模拟增益自适应和数字增益自适应以及固定数字增益。
		/*
		 * 固定数字增益模式最基础的增益模式也是 AGC 的核心，其他两种模式都是在此基础上扩展得到。主要是对信号进行固定增益的放大，
		 * 最大增益不超过设置的增益能力 compressionGaindB，结合 limiter 使用的时候上限不超过设置的目标音量 targetLevelDbfs。
		 */
		// WebRTC的AGC使用纯定点化实现
		WebRtcAgc_Init(agcHandle, minLevel, maxLevel, agcMode, fs);
		/* 在初始化函数里首先会进行数字域的初始化，设定一些参数的初始值。
		 * int32_t WebRtcAgc_InitDigital(DigitalAgc *stt, int16_t agcMode);
		 * 值得注意的是，这里面也初始化了近端信号和远端信号的VAD
		 * AGC的VAD是通过能量相关的阈值来判别语音信号的，有消息说最新版的WebRTCAGC采用RNN进行VAD判决
		 * WebRtcAgc_InitVad(&stt->vadNearend);
		 * WebRtcAgc_InitVad(&stt->vadFarend);
		 * 在初始化函数主体也会进行VAD的初始化，并且还会根据不同的模式对输入的minLevel和maxLevel进行缩放
		 * 如果选用kAgcModeAdaptiveDigital这个模式会自动设定为0和255
		 */

		WebRtcAgc_config_t agcConfig;
		agcConfig.compressionGaindB = 20;    // compressionGaindB是压缩增益  增益能力 表示音频最大的增益能力，如设置为 12dB，最大可以被提升 12dB
		agcConfig.limiterEnable = 1;    // limiterEnable是否使用limiter  压限器开关 一般与 targetLevelDbfs 配合使用，compressionGaindB 是调节小音量的增益范围，limiter 则是对超过 targetLevelDbfs 的部分进行限制，避免数据爆音。
		agcConfig.targetLevelDbfs = 3;    // targetLevelDbfs就是目标电平  目标音量 表示音量均衡结果的目标值，如设置为 1 表示输出音量的目标值为 - 1dB 经常设置成3 一般音量到达 -3dB 已经比较大了
		WebRtcAgc_set_config(agcHandle, agcConfig);

		infp = fopen(filename, "rb");
		int frameSize = 80;    // size_t samples = MIN(160, sampleRate / 100);
		pData = (short*)malloc(frameSize * sizeof(short));
		pOutData = (short*)malloc(frameSize * sizeof(short));

		outfp = fopen(outfilename, "wb");
		int len = frameSize * sizeof(short);
		int micLevelIn = 0;
		int micLevelOut = 0;
		while (TRUE)
		{
			memset(pData, 0, len);
			len = fread(pData, 1, len, infp);
			if (len > 0)
			{
				int inMicLevel = micLevelOut;
				int outMicLevel = 0;
				uint8_t saturationWarning;
				int nAgcRet = WebRtcAgc_Process(agcHandle, pData, NULL, frameSize, pOutData, NULL, inMicLevel, &outMicLevel, 0, &saturationWarning);
				if (nAgcRet != 0)
				{
					printf("failed in WebRtcAgc_Process\n");
					break;
				}
				micLevelIn = outMicLevel;
				fwrite(pOutData, 1, len, outfp);
			}
			else
			{
				break;
			}
		}
	} while (0);

	fclose(infp);
	fclose(outfp);
	free(pData);
	free(pOutData);
	WebRtcAgc_Free(agcHandle);
}

int _tmain(int argc, _TCHAR* argv[])
{
	WebRtcAgcTest("byby_8K_1C_16bit.pcm", "byby_8K_1C_16bit_agc.pcm", 8000);

	NoiseSuppression32("lhydd_1C_16bit_32K.PCM", "lhydd_1C_16bit_32K_ns.pcm", 32000, 1); // 浮点运算

	NoiseSuppression("gudao.pcm", "gudao_ns.pcm", 8000, 1); // 重新写了函数，可以实现8k 16k 32k采样率
	NoiseSuppression32("gudao.pcm", "gudao_ns1.pcm", 32000, 1); //参数只能设置成32k 但能跑8k的数据 效果没更改的函数好 但设置成8k没有高频
	//NoiseSuppression("byby_8K_1C_16bit.pcm", "byby_8K_1C_16bit_ns.pcm", 8000, 1);

	NoiseSuppressionX32("lhydd_1C_16bit_32K.PCM", "lhydd_1C_16bit_32K_nsx.pcm", 32000, 1); // 定点运算

	printf("声音增益，降噪结束...\n");
	getchar();
	return 0;
}