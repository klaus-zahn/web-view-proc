

#include "image_processing.h"


CImageProcessor::CImageProcessor(): m_counter(0), m_startButton(false), m_valueMin(20), m_valueMax(255), m_saturationMin(30) {
	for(uint32 i=0; i<3; i++) {
		/* index 0 is 3 channels and indicies 1/2 are 1 channel deep */
		m_proc_image[i] = new cv::Mat();
	}
}

CImageProcessor::~CImageProcessor() {
	for(uint32 i=0; i<3; i++) {
		delete m_proc_image[i];
	}
}

cv::Mat* CImageProcessor::GetProcImage(uint32 i) {
	if(2 < i) {
		i = 2;
	}
	return m_proc_image[i];
}

int CImageProcessor::DoProcess(cv::Mat* image) {

        if (!image) return (EINVALID_PARAMETER);

        int vmin, vmax, smin;

        vmin = m_valueMin;
        vmax = m_valueMax;
        smin = m_saturationMin;

        if (!image) return (EINVALID_PARAMETER);


        if (!m_startButton) {
            cv::subtract(cv::Scalar::all(255), *image,*m_proc_image[0]);
        } else {
            cv::add(cv::Scalar::all(100), *image,*m_proc_image[0]);
        }

        std::cout << "counter: " << m_counter << ", vmin: " << vmin << ", vmax: " << vmax << ", smin: " << smin << std::endl; 

        m_counter++;

        return (SUCCESS);
}









