// WebRtcAudioTest.cpp : �������̨Ӧ�ó������ڵ㡣
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

/* ����д�����ֲ����� */
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
					//����Ҫ�����������Ϊ��Ƶ���ݴ����Ӧ�ӿڣ���Ƶ����Ϊ�գ�ͬʱ��Ҫע�ⷵ������Ҳ�Ƿָ�Ƶ�͵�Ƶ���˴���Ƶ����Ϊ������
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
					//����Ҫ�����������Ϊ��Ƶ���ݴ����Ӧ�ӿڣ���Ƶ����Ϊ�գ�ͬʱ��Ҫע�ⷵ������Ҳ�Ƿָ�Ƶ�͵�Ƶ���˴���Ƶ����Ϊ������
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
					//������Ҫʹ���˲���������Ƶ���ݷָߵ�Ƶ���Ը�Ƶ�͵�Ƶ�ķ�ʽ���뽵�뺯���ڲ�
					WebRtcSpl_AnalysisQMF(shBufferIn, 320, shInL, shInH, filter_state1, filter_state12);

					//����Ҫ����������Ը�Ƶ�͵�Ƶ�����Ӧ�ӿڣ�ͬʱ��Ҫע�ⷵ������Ҳ�Ƿָ�Ƶ�͵�Ƶ
					if (0 == WebRtcNs_Process(pNS_inst, shInL, shInH, shOutL, shOutH))
					{
						short shBufferOut[320];
						//�������ɹ�������ݽ�����Ƶ�͵�Ƶ���ݴ����˲��ӿڣ�Ȼ���ý����ص�����д���ļ�
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
				//������Ҫʹ���˲���������Ƶ���ݷָߵ�Ƶ���Ը�Ƶ�͵�Ƶ�ķ�ʽ���뽵�뺯���ڲ�
				WebRtcSpl_AnalysisQMF(shBufferIn, 320, shInL, shInH, filter_state1, filter_state12);

				//����Ҫ����������Ը�Ƶ�͵�Ƶ�����Ӧ�ӿڣ�ͬʱ��Ҫע�ⷵ������Ҳ�Ƿָ�Ƶ�͵�Ƶ
				if (0 == WebRtcNs_Process(pNS_inst, shInL, shInH, shOutL, shOutH))
				{
					short shBufferOut[320];
					//�������ɹ�������ݽ�����Ƶ�͵�Ƶ���ݴ����˲��ӿڣ�Ȼ���ý����ص�����д���ļ�
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

/* ԭʼ��32k������ */
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
				//������Ҫʹ���˲���������Ƶ���ݷָߵ�Ƶ���Ը�Ƶ�͵�Ƶ�ķ�ʽ���뽵�뺯���ڲ�
				WebRtcSpl_AnalysisQMF(shBufferIn, 320, shInL, shInH, filter_state1, filter_state12);

				//����Ҫ����������Ը�Ƶ�͵�Ƶ�����Ӧ�ӿڣ�ͬʱ��Ҫע�ⷵ������Ҳ�Ƿָ�Ƶ�͵�Ƶ
				if (0 == WebRtcNsx_Process(pNS_inst, shInL, shInH, shOutL, shOutH))
				{
					short shBufferOut[320];
					//�������ɹ�������ݽ�����Ƶ�͵�Ƶ���ݴ����˲��ӿڣ�Ȼ���ý����ص�����д���ļ�
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
		int maxLevel = 255;    // minLevel��maxLevel�ֱ�����������Сֵ�����ֵ
		/*
		 * kAgcModeUnchanged,
		 * kAgcModeAdaptiveAnalog,  // ����Ӧģ��ģʽ
		 * kAgcModeAdaptiveDigital, // ����Ӧ��������ģʽ
		 * kAgcModeFixedDigital  // �̶���������ģʽ
		 */
		int agcMode = kAgcModeFixedDigital;    // 4��ģʽ��һ��ģʽ��ʲô�����ı䣬���ǻ�������������Ȼ����ģ����������Ӧ��������������Ӧ�Լ��̶��������档
		/*
		 * �̶���������ģʽ�����������ģʽҲ�� AGC �ĺ��ģ���������ģʽ�����ڴ˻�������չ�õ�����Ҫ�Ƕ��źŽ��й̶�����ķŴ�
		 * ������治�������õ��������� compressionGaindB����� limiter ʹ�õ�ʱ�����޲��������õ�Ŀ������ targetLevelDbfs��
		 */
		// WebRTC��AGCʹ�ô����㻯ʵ��
		WebRtcAgc_Init(agcHandle, minLevel, maxLevel, agcMode, fs);
		/* �ڳ�ʼ�����������Ȼ����������ĳ�ʼ�����趨һЩ�����ĳ�ʼֵ��
		 * int32_t WebRtcAgc_InitDigital(DigitalAgc *stt, int16_t agcMode);
		 * ֵ��ע����ǣ�������Ҳ��ʼ���˽����źź�Զ���źŵ�VAD
		 * AGC��VAD��ͨ��������ص���ֵ���б������źŵģ�����Ϣ˵���°��WebRTCAGC����RNN����VAD�о�
		 * WebRtcAgc_InitVad(&stt->vadNearend);
		 * WebRtcAgc_InitVad(&stt->vadFarend);
		 * �ڳ�ʼ����������Ҳ�����VAD�ĳ�ʼ�������һ�����ݲ�ͬ��ģʽ�������minLevel��maxLevel��������
		 * ���ѡ��kAgcModeAdaptiveDigital���ģʽ���Զ��趨Ϊ0��255
		 */

		WebRtcAgc_config_t agcConfig;
		agcConfig.compressionGaindB = 20;    // compressionGaindB��ѹ������  �������� ��ʾ��Ƶ��������������������Ϊ 12dB�������Ա����� 12dB
		agcConfig.limiterEnable = 1;    // limiterEnable�Ƿ�ʹ��limiter  ѹ�������� һ���� targetLevelDbfs ���ʹ�ã�compressionGaindB �ǵ���С���������淶Χ��limiter ���ǶԳ��� targetLevelDbfs �Ĳ��ֽ������ƣ��������ݱ�����
		agcConfig.targetLevelDbfs = 3;    // targetLevelDbfs����Ŀ���ƽ  Ŀ������ ��ʾ������������Ŀ��ֵ��������Ϊ 1 ��ʾ���������Ŀ��ֵΪ - 1dB �������ó�3 һ���������� -3dB �Ѿ��Ƚϴ���
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

	NoiseSuppression32("lhydd_1C_16bit_32K.PCM", "lhydd_1C_16bit_32K_ns.pcm", 32000, 1); // ��������

	NoiseSuppression("gudao.pcm", "gudao_ns.pcm", 8000, 1); // ����д�˺���������ʵ��8k 16k 32k������
	NoiseSuppression32("gudao.pcm", "gudao_ns1.pcm", 32000, 1); //����ֻ�����ó�32k ������8k������ Ч��û���ĵĺ����� �����ó�8kû�и�Ƶ
	//NoiseSuppression("byby_8K_1C_16bit.pcm", "byby_8K_1C_16bit_ns.pcm", 8000, 1);

	NoiseSuppressionX32("lhydd_1C_16bit_32K.PCM", "lhydd_1C_16bit_32K_nsx.pcm", 32000, 1); // ��������

	printf("�������棬�������...\n");
	getchar();
	return 0;
}