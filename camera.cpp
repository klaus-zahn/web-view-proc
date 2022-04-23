
#include "camera.h"
#include <fstream>


CCamera::CCamera() : m_img(NULL), m_frame_buffer_ids(NULL), m_frame_buffers(NULL)
	, m_bRoi_changed(false), m_color_type(ColorType_gray), m_perspective(0) {}

CCamera::~CCamera() {
	if(m_frame_buffer_ids) delete[](m_frame_buffer_ids);
	if(m_frame_buffers) delete[](m_frame_buffers);
	if(m_img) {delete m_img;m_img = NULL;}
}

OSC_ERR CCamera::Init(const ROI& region_of_interest, uint8 buffer_count) {
	
	
	if(buffer_count==0 || region_of_interest.width==0
			|| region_of_interest.height==0) return(EINVALID_PARAMETER);
	OscAssert_w(region_of_interest.width%4==0, "region_of_interest.width must be a multiple of 4");
	
	OSC_ERR err;
	m_roi=region_of_interest;
	
	/* create the frame buffers */
	if(m_frame_buffers) delete[](m_frame_buffers);
	const unsigned int buffer_size=3*region_of_interest.width*region_of_interest.height;
	const unsigned int buffer_size_al=AlignSize(buffer_size);
	m_frame_buffers=new uint8_t[buffer_count*buffer_size_al+PICTURE_ALIGNMENT];
	uint8_t* frame_buffers_al=(uint8_t*)AlignPicture((uint8*)m_frame_buffers);
	m_buffer_count=buffer_count;
	
	/* Set the camera registers to sane default values. */
	OscCamPresetRegs();
	if((err=OscCamSetAreaOfInterest(region_of_interest.low_x, region_of_interest.low_y
			, region_of_interest.width, region_of_interest.height))!=SUCCESS) 
		return(err);
	
	/* Set up frame buffers cached memory. */
	for(int i=0; i<buffer_count; ++i) {
		if((err=OscCamSetFrameBuffer(i, sizeof(uint8_t)*buffer_size
				, frame_buffers_al+i*buffer_size_al, true))!=SUCCESS) 
			return(err);
	}
	
	/* Create multibuffer indexes if needed */
	if(m_frame_buffer_ids) delete[](m_frame_buffer_ids);
	m_frame_buffer_ids=NULL;
	if(buffer_count>1) {
		m_frame_buffer_ids=new uint8[buffer_count];
		for(int i=0; i<buffer_count; ++i)
			m_frame_buffer_ids[i]=i;
		
		if((err=OscCamCreateMultiBuffer(buffer_count, m_frame_buffer_ids))!=SUCCESS) 
			return(err);
	}
	
	return(SUCCESS);
}

uint8* CCamera::AlignPicture(const uint8* pic) {
  return((uint8*)(((ptrdiff_t)pic + (ptrdiff_t)PICTURE_ALIGNMENT-1) & -(ptrdiff_t)PICTURE_ALIGNMENT));
}

uint32 CCamera::AlignSize(uint32 size) {
	return((size + PICTURE_ALIGNMENT-1) & -PICTURE_ALIGNMENT);
}




cv::Mat* CCamera::ReadLatestPicture() {
	
	uint8* pic_data = NULL;
#ifdef OSC_HOST
	/* wait for the picture to be captured -> otherwise it could run too fast on the host */
	OscCamReadPicture(m_buffer_count>1 ? OSC_CAM_MULTI_BUFFER : 0, &pic_data, 0, 0);
#endif
	if(OscCamReadLatestPicture(&pic_data)==SUCCESS) {
		return(HandlePictureColoringAndSize(pic_data));
	}
	return(NULL);
}

cv::Mat* CCamera::ReadPicture( uint16 max_age, uint16 timeout) {
	
	uint8* pic_data = NULL;
	if(OscCamReadPicture(m_buffer_count>1 ? OSC_CAM_MULTI_BUFFER : 0, &pic_data, max_age, timeout)==SUCCESS) {
		return(HandlePictureColoringAndSize(pic_data));

	}

	return(NULL);
}

cv::Mat* CCamera::HandlePictureColoringAndSize(uint8* pic_data) {
	
	if(m_color_type == ColorType_none || !pic_data) return(0);
        
        if(m_color_type == ColorType_debayered) {

		AdjustImageHeader(m_img, 3);
		
		*m_img = cv::Mat(m_img->rows, m_img->cols, CV_8UC3, pic_data).clone();
	} else {
		// 1 channel
		AdjustImageHeader(m_img, 1);
                
                cv::Mat col_img = cv::Mat(m_img->rows, m_img->cols, CV_8UC3, pic_data);
                
                cv::cvtColor(col_img, *m_img, cv::COLOR_RGB2GRAY);
                //cv::imwrite("orig.png", col_img);
		
/*
		static int count = 0;

		if(count < 5) {
			std::ofstream o("out.raw", std::ios::out | std::ios::binary | std::ios::trunc);
			o.write((const char*)pic_data, 2*OSC_CAM_MAX_IMAGE_HEIGHT*OSC_CAM_MAX_IMAGE_WIDTH);
			o.close();
			printf("file written\n");
		}
		count++;*/
	}
        
        //cv::imwrite("gray.png", *m_img);

	return(m_img);
}

OSC_ERR CCamera::CapturePicture() {
	OSC_ERR ret;
	
	ret=OscCamSetupCapture( m_buffer_count>1 ? OSC_CAM_MULTI_BUFFER : 0 );
	if(ret==SUCCESS) ret=OscGpioTriggerImage();
	
	return(ret);
}


void CCamera::AdjustImageHeader(cv::Mat*& img, int channel_count) {
	
	if(!(img) || m_bRoi_changed || img->channels()!=channel_count) {
		m_bRoi_changed=false;
		
		if(img) delete img;
                if(channel_count==1){
                    img=new cv::Mat(cv::Size(m_roi.width, m_roi.height), CV_8UC1);
                } else if(channel_count==3){
                    img=new cv::Mat(cv::Size(m_roi.width, m_roi.height), CV_8UC3);
                } else {
                    OscLog(ERROR, "channel_count not supported");
                }
		
	}
}


ColorType CCamera::getAppropriateColorType() { 
	
	struct OscSystemInfo * pInfo;
	if(OscCfgGetSystemInfo(&pInfo)==SUCCESS) {
            return(pInfo->hardware.imageSensor.hasBayernPattern ? ColorType_debayered : ColorType_gray);
	}
        return ColorType_gray;//default is grey
}


void CCamera::setAutoExposure(bool bEnabled) {
	uint16 reg_val;
	OscCamGetRegisterValue(REG_AEC_AGC_ENABLE, &reg_val);
	OscCamSetRegisterValue(REG_AEC_AGC_ENABLE, (reg_val & ~0x1) | (uint16)bEnabled);
}

bool CCamera::getAutoExposure() const {
	uint16 reg_val;
	OscCamGetRegisterValue(REG_AEC_AGC_ENABLE, &reg_val);
	return(reg_val & 0x1);
}









