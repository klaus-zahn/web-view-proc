

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
        
    if(image->channels() > 1) {

        int vmin, vmax, smin;

        vmin = m_valueMin;
        vmax = m_valueMax;
        smin = m_saturationMin;

        if (!image) return (EINVALID_PARAMETER);
        

        *m_proc_image[0] = image->clone();

        if (!m_startButton) {
            
            
            int width = 200;
            int height = 200;

            char msg[100];
                        
            cv::Mat hsv = image->clone();
            cv::Mat roi;
            
            m_trackWindow = cv::Rect((image->cols-width)/2, (image->rows-height)/2, width, height);
            cv::rectangle( *m_proc_image[0], m_trackWindow, cv::Scalar(0,0,255), 3);

            cv::cvtColor(*image, hsv, cv::COLOR_BGR2HSV );
            
            roi = hsv(m_trackWindow); 
            //ZaK: the following statement leads to a compiler warning because roi_mean is not used
            //good practice is usage of #if statement for debug output
            cv::Scalar roi_mean = cv::mean(roi);
            
            // snprintf(msg, 100, "mean values are: h = %3.1f, s = %3.1f, v = %3.1f", roi_mean[0]*2, roi_mean[1]*2, roi_mean[2]*2);
            //std::cout << msg << std::endl;
            
            cv::Mat hue;
            int ch[] = {0,0};
            hue.create(roi.size(), roi.depth());
            cv::mixChannels(&roi, 1, &hue, 1, ch, 1);
            
            int hsize = 16;
            float hranges[] = {0, 180};
            const float* phranges = hranges;
            
            cv::calcHist(&hue, 1, 0, hue, m_hist, 1, &hsize, &phranges);
            cv::normalize(m_hist, m_hist, 0,255, cv::NORM_MINMAX);
            
            // snprintf(msg, 100, "%d,", m_hist);
            // std::cout << m_hist << std::endl;

            //ZaK: why do you copy the output image?
            *m_proc_image[1] = *m_proc_image[0];
            
        } else {
            
            cv::Mat hsv = image->clone();
            cv::cvtColor(*image, hsv, cv::COLOR_BGR2HSV);
            
            cv::Mat hue;
            int ch[] = {0,0};
            hue.create(hsv.size(), hsv.depth());
            cv::mixChannels(&hsv, 1, &hue, 1, ch, 1);
            
            
            cv::Mat backproj;
            float hranges[] = {0, 180};
            const float * phranges = hranges;
            cv::calcBackProject(&hue, 1, 0, m_hist, backproj, &phranges);
            
            cv::Mat mask;
            cv::inRange(hsv, cv::Scalar(0, smin, MIN(vmin,vmax)), cv::Scalar(180, 256, MAX(vmin, vmax)), mask);
            backproj &= mask;
            
            //ZaK: good practice to use debug output :-)
            *m_proc_image[1] = backproj;
            *m_proc_image[2] = mask;

            cv::rectangle( *m_proc_image[0], m_trackWindow, cv::Scalar(0,255,255), 1);
            
            
            cv::RotatedRect trackBox = cv::CamShift(backproj, m_trackWindow, cv::TermCriteria(cv::TermCriteria::EPS | cv::TermCriteria::COUNT, 10, 1));
            cv::ellipse(*m_proc_image[0], trackBox, cv::Scalar(0, 0, 255), 3, cv::LINE_AA);
        }

        // std::cout << "counter: " << m_counter << ", vmin: " << vmin << ", vmax: " << vmax << ", smin: " << smin << std::endl; 

        m_counter++;
        
    } else {
        std::cout << "Bitte auf Farbbild umstellen" << std::endl;
    }

        return (SUCCESS);
}









