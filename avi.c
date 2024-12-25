/*
Based on an old homebrew created by "jonny"

These are long dead but I include them:

Homepage: http://jonny.leffe.dnsalias.com
E-mail:   jonny@leffe.dnsalias.com

*/


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <pspsdk.h>
#include <pspaudiocodec.h>
#include <pspgu.h>
#include <pspthreadman.h>
#include <pspaudio.h>
#include <pspaudio_kernel.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <psppower.h>
#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspmpeg.h>
#include <pspjpeg.h>
#include <pspkernel.h>
#include <pspsdk.h>
#include <psprtc.h>
#include <psputility_avmodules.h>


void *fbp0 = 0;
void *fbp1 = 0;
void *zbp  = 0;
void *framebuffer = 0;
static unsigned int __attribute__((aligned(16))) list[262144];

u32 AVI_Streams = 0;
u32 AVI_TotalFrames = 0;
u32 AVI_TotalAACFrames = 0;
u64 AVI_Last_Tick = 0;
u64 AVI_Current_Tick = 0;
u32 AVI_Frame = 0;
u32 AVI_AAC_Frame = 0;
u64 AVI_MicroSecPerFrame = 0;
u32 AVI_Loop = 0;
u32 AVI_Width = 0;
u32 AVI_Height = 0;
u32 AVI_WidthP2 = 0;
u32 AVI_HeightP2 = 0;
SceUID AVI_H264_thread;
SceUID AVI_AAC_thread;
int AVI_Audio_SemaID;
int AVI_Video_SemaID;
int _AVI_FILE = 0;
int AVI_AAC_File = 0;
int AVI_AAC_Handle = 0;
u32 AVI_idx1_size = 0;
u32 AVI_start = 0;
u32 AVI_relative_offset = 0;

void *AVI_a;
void *AVI_b;

typedef struct{
	u32 name;
	u32 flags;
	u32 offset;
	u32 size;
	u32 name1;
	u32 flags1;
	u32 offset1;
	u32 size1;
} AVI_chunk;

AVI_chunk c;

unsigned long aac_codec_buffer[2048] __attribute__((aligned(64)));
u8 aac_data_buffer0[2048] __attribute__((aligned(64)));
u8 aac_data_buffer1[2048] __attribute__((aligned(64)));
short pcm_output_buffer[2][4096] __attribute__((aligned(64)));
int pcm_output_index = 0;
SceUID aac_handle;
u32 aac_sample_per_frame;
u8 aac_getEDRAM;
u32 aac_channels;
u32 aac_samplerate;


//H264
//////

struct SceMpegLLI{
	ScePVoid pSrc;
	ScePVoid pDst;
	ScePVoid Next;
	SceInt32 iSize;
};

struct avc_struct{
	int mpeg_init;
	ScePVoid mpeg_data;
	int      mpeg_ringbuffer_construct;
	int      mpeg_create;
	int      mpeg_format;
	int      mpeg_width;

	SceMpegRingbuffer  mpeg_ringbuffer;
	SceMpeg            mpeg;
	ScePVoid           mpeg_es;
	struct SceMpegLLI *mpeg_lli;
	SceMpegAu          mpeg_au;
};

struct avc_struct v_decode;
u32 a_offset;
u32 b_offset;
SceInt64 result;
int audio_channel;
SceInt32 unused;
SceInt32 sceMpegbase_BEA18F91(struct SceMpegLLI *p);

#define H264_Buffer_Frames			6
#define H264_max_frame_size			0xFFFF	//For I frames, x4 for 720x480
#define H264_buffer_size			H264_max_frame_size	* H264_Buffer_Frames

//We just need a buffer of raw h264 frames (and their sizes), each aligned to 64 bytes,
//then "sceMpegAvcDecode" will do everything for us
unsigned char *H264_RingBuffer;//= (void*)(0x4198000);
//I think this replaces "sceMpegGetAvcAu" from sony PMF (video) player samples
//It sets the frame offset inside the buffer to be decoded by "sceMpegAvcDecode"
static void CopyAu2Me(struct avc_struct *p, void *source_buffer, int size){
	u32 MEAVCBUF = 0x4a000;//MEDIA_ENGINE_AVC_BUFFER
	u32 DMABLOCK = 4095;
	void *destination_buffer = (void *) MEAVCBUF;
	unsigned int i = 0;
	while (1){
		p->mpeg_lli[i].pSrc = source_buffer;
		p->mpeg_lli[i].pDst = destination_buffer;
		if (size > DMABLOCK){
			p->mpeg_lli[i].iSize = DMABLOCK;
			p->mpeg_lli[i].Next  = &p->mpeg_lli[i + 1];
			source_buffer      += DMABLOCK;
			destination_buffer += DMABLOCK;
			size               -= DMABLOCK;
			i                  ++;
		} else {
			p->mpeg_lli[i].iSize = size;
			p->mpeg_lli[i].Next  = 0;
			break;
		}
	}
	sceKernelDcacheWritebackInvalidateAll();
	sceMpegbase_BEA18F91(p->mpeg_lli);
}

void h264_close(struct avc_struct *p){
	if (p->mpeg_lli != 0) free(p->mpeg_lli);
	if (p->mpeg_es != 0) sceMpegFreeAvcEsBuf(&p->mpeg, p->mpeg_es);
	//if (p->mpeg_create == 0) sceMpegDelete(&p->mpeg);
	if (!(p->mpeg_ringbuffer_construct != 0)) sceMpegRingbufferDestruct(&p->mpeg_ringbuffer);
	if (!(p->mpeg_init != 0)) sceMpegFinish();
	if (p->mpeg_data != 0) free(p->mpeg_data);
}

int Set_H264_Decoder(struct avc_struct *p, unsigned int maximum_frame_size, int bufwidth, int format){
	int mod_64 = 0;
	u32 DMABLOCK = 4095;
	p->mpeg_init = -1;
	p->mpeg_data = 0;
	p->mpeg_ringbuffer_construct = -1;
	p->mpeg_create = -1;
	p->mpeg_es = 0;
	p->mpeg_lli = 0;
	p->mpeg_format = -1;
	p->mpeg_format = format;
	p->mpeg_width = bufwidth;
	sceMpegInit();
	int size = sceMpegQueryMemSize(0);
	
	//Malloc 64 aligned
	mod_64 = size & 0x3f;
	if (mod_64 != 0) size += 64 - mod_64;
	p->mpeg_data = memalign(64, size);
	
	p->mpeg_ringbuffer_construct = sceMpegRingbufferConstruct(&p->mpeg_ringbuffer, 0, 0, 0, 0, 0);
	p->mpeg_create = sceMpegCreate(&p->mpeg, p->mpeg_data, size, &p->mpeg_ringbuffer, p->mpeg_width, 0, 0);
	
	//Malloc 64 aligned
	H264_RingBuffer = memalign(64,H264_buffer_size);
	
	SceMpegAvcMode avc_mode;
	avc_mode.iUnk0 = -1;
	avc_mode.iPixelFormat = p->mpeg_format;
	p->mpeg_format = avc_mode.iPixelFormat;
	p->mpeg_es = sceMpegMallocAvcEsBuf(&p->mpeg);
	unsigned int maximum_number_of_blocks = (maximum_frame_size + DMABLOCK - 1) / DMABLOCK;
	
	//Malloc 64 aligned
	size = sizeof(struct SceMpegLLI) * maximum_number_of_blocks;
	mod_64 = size & 0x3f;
	if (mod_64 != 0) size += 64 - mod_64;
	p->mpeg_lli = memalign(64, size);
	
	memset(&p->mpeg_au, -1, sizeof(SceMpegAu));
	p->mpeg_au.iEsBuffer = 1;
	return 1;
}

int Set_AAC_Decoder(){
	memset(aac_codec_buffer, 0, sizeof(aac_codec_buffer));
	if (sceAudiocodecCheckNeedMem(aac_codec_buffer, 0x1003) < 0) return 0;
	if (sceAudiocodecGetEDRAM(aac_codec_buffer, 0x1003) < 0 ) return 0;
	aac_getEDRAM = 1;
	aac_codec_buffer[10] = 44100;
	if (sceAudiocodecInit(aac_codec_buffer, 0x1003) < 0) return 0;
	audio_channel = sceAudioChReserve(0,1024, PSP_AUDIO_FORMAT_STEREO); 
	return 1;
}

int Set_MP3_Decoder(){
	memset(aac_codec_buffer, 0, sizeof(aac_codec_buffer));
	if ( sceAudiocodecCheckNeedMem(aac_codec_buffer, 0x1002) < 0 ) return -1;
	if ( sceAudiocodecGetEDRAM(aac_codec_buffer, 0x1002) < 0 ) return -1;
	if ( sceAudiocodecInit(aac_codec_buffer, 0x1002) < 0 ) return -1;
	
	audio_channel = sceAudioChReserve(0,0x1000, PSP_AUDIO_FORMAT_STEREO); 
    
	return 1;
}

//Decode threads
int decode_audio(){
	while(1){
		sceKernelWaitSema(AVI_Audio_SemaID, 1, 0);
		aac_codec_buffer[6] = (unsigned long)aac_data_buffer0;
		aac_codec_buffer[7] = c.size;
		aac_codec_buffer[8] = (unsigned long)pcm_output_buffer[pcm_output_index&1];
		aac_codec_buffer[9] = 1024;
		if ( sceAudiocodecDecode(aac_codec_buffer, 0x1003) < 0){
			//It did not decode any sound, or corrupt
			sceKernelDelayThread(100);//removed clicks on real psp
			memset(&pcm_output_buffer[pcm_output_index&1], 0, 2048);
		} else {
			//It decoded sound, play it
			sceKernelDelayThread(100);//removed clicks on real psp
			sceAudioOutputBlocking(audio_channel, PSP_AUDIO_VOLUME_MAX, pcm_output_buffer[pcm_output_index&1]);
			pcm_output_index++;
		}
	}
	return 0;
}

int decode_MP3(){
	while(1){
		aac_codec_buffer[6] = (unsigned long)aac_data_buffer0;
		aac_codec_buffer[8] = (unsigned long)pcm_output_buffer[pcm_output_index&1];
		aac_codec_buffer[7] = aac_codec_buffer[10] = c.size;
		aac_codec_buffer[9] = 0x1200;
    
		if ( sceAudiocodecDecode(aac_codec_buffer, 0x1002) < 0 )
			memset(aac_codec_buffer, 0, 0x1200);
		sceAudioOutputBlocking(audio_channel, PSP_AUDIO_VOLUME_MAX, pcm_output_buffer[pcm_output_index&1]);
		pcm_output_index++;
	}
	return 0;
}

int decode_video(){
	while(1){
		sceKernelWaitSema(AVI_Video_SemaID, 1, 0);
		void *destination_buffer = (void*)(0x04000000+framebuffer);
		CopyAu2Me(&v_decode, &H264_RingBuffer[a_offset],H264_max_frame_size);
		v_decode.mpeg_au.iAuSize = H264_max_frame_size;
		int result = sceMpegAvcDecode(&v_decode.mpeg,&v_decode.mpeg_au,v_decode.mpeg_width,&destination_buffer,&unused);
		if (result == 0){//It decoded a frame, show it
			sceKernelDcacheWritebackInvalidateAll();
			sceDisplayWaitVblankStart();
			framebuffer = sceGuSwapBuffers();
		}
		//sceKernelDelayThread(10000);
	}
	return 0;
}


//next power of two (from pmp player)
u32 next_pow2(u32 v){
	v-=1;v|=(v>>1);v|=(v>>2);v|=(v>>4);v|=(v>>8);v|=(v>>16);return(v+1);
}

int Load_Play_AVI(char *path,u32 button){
	int current_video_frame = 0;
	int loop_buffer = 0;
	int loop_frames = H264_Buffer_Frames/2;
	u32 buffer_start_offset = H264_buffer_size/2;
	a_offset = buffer_start_offset;
	b_offset = 0;
	
	u8 movi = 0;
	u8 idx1 = 0;
	u32 WB = 0x62773130;//aac chunk 01wb
	u32 DC = 0x63643030;//mjpeg frame 00dc
	u32 MOVI = 0x69766f6D;//movi chunk
	u32 INFO = 0x4F464E49;//info chunk
	u32 JUNK = 0x4B4E554A;//junk block
	u32 IDX = 0x31786469;//index block   

	u32 movi_offset = 0;
	u32 movi_size = 0;
	AVI_relative_offset = 0;
	u32 av1 = 0;u32 size = 0; u32 av3 = 0;
	u32 hdrl_size = 0;
	u32 avih_size = 0;
	
	_AVI_FILE = sceIoOpen(path, PSP_O_RDONLY, 0777 );

	if (_AVI_FILE<0) return 0;
	AVI_Frame = 0;
	AVI_AAC_Frame = 0;
	sceIoRead(_AVI_FILE,&av1,4);
	sceIoRead(_AVI_FILE,&size,4);
	sceIoRead(_AVI_FILE,&av3,4);
	if (av1 != 0x46464952) return 0;//RIFF
	if (av3 != 0x20495641) return 0;//AVI_
	sceIoLseek32(_AVI_FILE,0x04,SEEK_CUR);	
	sceIoRead(_AVI_FILE,&hdrl_size,4);
	movi_offset = hdrl_size+16+8+4;
	sceIoLseek32(_AVI_FILE,0x08,SEEK_CUR);
	sceIoRead(_AVI_FILE,&avih_size,4);
	if (avih_size != 0x38) return 0;//RIFF
	sceIoRead(_AVI_FILE,&AVI_MicroSecPerFrame,4);
	sceIoLseek32(_AVI_FILE,12,SEEK_CUR);
	sceIoRead(_AVI_FILE,&AVI_TotalFrames,4);
	sceIoLseek32(_AVI_FILE,4,SEEK_CUR);
	sceIoRead(_AVI_FILE,&AVI_Streams,4);
	sceIoLseek32(_AVI_FILE,4,SEEK_CUR);
	sceIoRead(_AVI_FILE,&AVI_Width,4);
	sceIoRead(_AVI_FILE,&AVI_Height,4);
	
	//check size
	if((AVI_Width<769)&&(AVI_Height<513)){
		AVI_WidthP2 =  (AVI_Width);
		AVI_HeightP2 = next_pow2(AVI_Height);
		if (AVI_Width > 512) {
			AVI_WidthP2 = 768;
			AVI_HeightP2 = 512;
		}
	} else return 0;

	//look for movi
	u32 offset2 = sceIoLseek32(_AVI_FILE,movi_offset,SEEK_SET);
	while (!movi){
		int b = sceIoRead(_AVI_FILE,&c.name,4);
		if (b == 0) {
			return 0;
		}
		if (c.name == MOVI) {
			movi = 1;
			movi_offset = sceIoLseek(_AVI_FILE,0, SEEK_CUR);
			break;
		}
		if (c.name == INFO) {
			sceIoLseek32(_AVI_FILE,offset2-4,SEEK_SET);
			sceIoRead(_AVI_FILE,&size,4);
			sceIoLseek32(_AVI_FILE,size,SEEK_CUR);
			offset2 = sceIoLseek(_AVI_FILE,0, SEEK_CUR);
		}
		if (c.name == JUNK) {
			sceIoRead(_AVI_FILE,&size,4);
			sceIoLseek32(_AVI_FILE,size+8,SEEK_CUR);
		}
	}
	
	//look for idx1
	sceIoLseek32(_AVI_FILE,movi_offset-8,SEEK_SET);
	sceIoRead(_AVI_FILE,&movi_size,4);
	sceIoLseek32(_AVI_FILE,movi_offset+movi_size-4,SEEK_SET);
	sceIoRead(_AVI_FILE,&c.name,4);
	if (c.name == IDX) idx1 = 1;
	if (idx1) {
		//go to idx1
		sceIoLseek32(_AVI_FILE,movi_offset+movi_size,SEEK_SET);
		sceIoRead(_AVI_FILE,&AVI_idx1_size,4);
		//check if first offset is absolute or relative (to movi)
		sceIoLseek32(_AVI_FILE,8,SEEK_CUR);
		sceIoRead(_AVI_FILE,&c.offset,4);//read first offset
		if (c.offset == 4) AVI_relative_offset = movi_offset+4;
		//Go bak to read all chunks
		AVI_start = sceIoLseek32(_AVI_FILE,movi_offset+movi_size+4,SEEK_SET);
		
		//Set video decoder
		Set_H264_Decoder(&v_decode, 1024, 512, GU_PSM_8888);	
		//Set audio decoder
		Set_AAC_Decoder();
		
		//Start video thread
		AVI_Video_SemaID = sceKernelCreateSema("can read video", 0, 0, 1, 0);
		if(!AVI_Video_SemaID) return 0;
		AVI_H264_thread = sceKernelCreateThread("decode video",decode_video, 0x8, 0x10000, 0, 0);
		if (AVI_H264_thread < 0) return 0;
		sceKernelStartThread(AVI_H264_thread, 4, &AVI_a);
		
		//Start audio thread
		AVI_Audio_SemaID = sceKernelCreateSema("can read audio", 0, 0, 1, 0);
		if(!AVI_Audio_SemaID) return 0;
		AVI_AAC_thread = sceKernelCreateThread("decode audio",decode_audio, 0xF, 0x10000, 0, 0);
		if (AVI_AAC_thread < 0) return 0;
		sceKernelStartThread(AVI_AAC_thread, 4, &AVI_b);
		
		//Start reader (main thread)
		u32 size = 0;
		int First_frame = 1;
		AVI_Last_Tick = 0;
		int audio_frame = 0;
		while(size != AVI_idx1_size){
			sceRtcGetCurrentTick(&AVI_Current_Tick);
			if((AVI_Current_Tick-AVI_Last_Tick > AVI_MicroSecPerFrame) || First_frame){
				read_again:
				AVI_Last_Tick = AVI_Current_Tick;
				First_frame = 0;
				sceKernelSignalSema(AVI_Video_SemaID, 0);
				sceKernelSignalSema(AVI_Audio_SemaID, 0);
				sceIoLseek32(_AVI_FILE,AVI_start+size,SEEK_SET);
				sceIoRead(_AVI_FILE,&c,16);
				if (c.name == DC){
					//Read frame and add to buffer
					u32 frame_offset = c.offset+AVI_relative_offset;
					u32 frame_size = c.size;
					//Fill ring buffer
					sceIoLseek32(_AVI_FILE,frame_offset,SEEK_SET);
					sceIoRead(_AVI_FILE,&H264_RingBuffer[a_offset], frame_size);
					memcpy(&H264_RingBuffer[b_offset],&H264_RingBuffer[a_offset],frame_size);
	
					a_offset += H264_max_frame_size;
					b_offset += H264_max_frame_size;
					current_video_frame++;
					loop_buffer++;
					//Update buffer pointers
					if (loop_buffer == loop_frames) {
						loop_buffer = 0;
						a_offset = buffer_start_offset;
						b_offset = 0;
					}
					sceKernelSignalSema(AVI_Video_SemaID, 1);//Tell ME to decode a frame
					size+=16;
				}
				if (c.name == WB) {
					sceIoLseek32(_AVI_FILE,c.offset+AVI_relative_offset, PSP_SEEK_SET);
					sceIoRead(_AVI_FILE,aac_data_buffer0,c.size);
					sceKernelSignalSema(AVI_Audio_SemaID, 1);//Tell ME to decode an audio AAC frame
					size+=16;
					goto read_again;
					//sceIoWaitAsync(_AVI_FILE,&result);
				}
				SceCtrlData pad;
				sceCtrlReadBufferPositive(&pad, 1);
				if (pad.Buttons & button) {
					sceKernelSignalSema(AVI_Video_SemaID, 0);
					sceKernelSignalSema(AVI_Audio_SemaID, 0);
					sceKernelDelayThread(10000);
					break;
				}
			}
		}
	} else {
		return 0;
	}
 
	h264_close(&v_decode);
	sceIoClose(_AVI_FILE);
	//they will never end, so we kill them
	//sceKernelWaitThreadEnd(AVI_H264_thread, 0);
	//sceKernelWaitThreadEnd(AVI_AAC_thread, 0);
	sceKernelDeleteThread(AVI_H264_thread);
	sceKernelDeleteThread(AVI_AAC_thread);
	if (aac_getEDRAM) sceAudiocodecReleaseEDRAM(aac_codec_buffer);
	return 1;
}


PSP_MODULE_INFO("H264_Test", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);
PSP_HEAP_SIZE_KB(-1024);

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)

int main(int argc, char **argv){

	//Controls
	sceCtrlSetSamplingCycle(0);

	// setup
	fbp0 = (void*)0;
	fbp1 = (void*)0x88000;
	zbp = (void*)0x110000;

	framebuffer = fbp1;
	pspDebugScreenInit();
	
	sceGuInit();
	sceGuStart(GU_DIRECT,list);
	sceGuDrawBuffer(GU_PSM_8888,fbp1,BUF_WIDTH);
	sceGuDispBuffer(SCR_WIDTH,SCR_HEIGHT,fbp0,BUF_WIDTH);
	sceGuDepthBuffer(zbp,BUF_WIDTH);
	sceGuOffset(2048 - (SCR_WIDTH/2),2048 - (SCR_HEIGHT/2));
	sceGuViewport(2048,2048,SCR_WIDTH,SCR_HEIGHT);
	sceGuDepthRange(65535,0);
	sceGuScissor(0,0,SCR_WIDTH,SCR_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuFrontFace(GU_CW);
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
	sceGuFinish();
	sceGuSync(0,0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(1);
	
	pspDebugScreenInit();
	
	sceUtilityLoadAvModule(PSP_AV_MODULE_AVCODEC);
	sceUtilityLoadAvModule(PSP_AV_MODULE_MPEGBASE);// Requires PSP_AV_MODULE_AVCODEC loading first
	sceUtilityLoadAvModule(PSP_AV_MODULE_MP3);
	
	Load_Play_AVI("psp_video.avi",PSP_CTRL_TRIANGLE);//Exit pressing triangle

	sceKernelExitGame();
	return 0;
}
