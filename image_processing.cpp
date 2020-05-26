

#include "image_processing.h"
#include "opencv/arm-obj_rel/include/opencv2/core/types.hpp"
#include "opencv/arm-obj_rel/include/opencv2/core.hpp"


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

        int width =50;
        int height = 50;
        
        char msg[100];
        
        int vmin, vmax, smin;

        vmin = m_valueMin;
        vmax = m_valueMax;
        smin = m_saturationMin;


        *m_proc_image[0]=image->clone();//!!!
        
        if (image->channels() > 1) 
        {
            if (!m_startButton) 
            {
                //Initialisierung
                //ZaK: remove obsolete code 
                //cv::subtract(cv::Scalar::all(255), *image,*m_proc_image[0]);

                //Schritt1:
                //ZaK: you clone image twice (-> l. 44)
                *m_proc_image[0] = image->clone();

                m_trackWindow = cv::Rect((image->cols - width) / 2, (image ->rows - height) / 2, width, height);
                cv::rectangle(*m_proc_image[0], m_trackWindow, cv::Scalar(0, 0, 255), 3, cv::LINE_AA);

                cv::Mat hsv;
                cv::cvtColor(*image, hsv, cv::COLOR_BGR2HSV);

                cv::Mat roi = hsv(m_trackWindow);
                cv::Scalar roi_mean = cv::mean(roi);

                snprintf(msg, 100, "mean values are: h = %3.1f, s = %3.1f, v = %3.1f", roi_mean[0], roi_mean[1], roi_mean[2]);
                cv::putText(*m_proc_image[0], msg, cv::Point(10, 20), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 255));
                //std::cout << msg << std::endl;

                //Schritt2: automatische bestimmung von hue im Rechteck
                //+ mit slider kanäle helligkeit+sättigung festlegen
                cv::Mat hue;
                int ch[] = {0, 0};
                hue.create(roi.size(), roi.depth());
                cv::mixChannels(&roi, 1, &hue, 1, ch, 1);


                //Schritt 3: aus Farbkanal des Rechtecks Hist bestimmen
                int hsize = 16;
                float hranges[] = {0, 180};
                const float* phranges = hranges;

                cv::calcHist(&hue, 1, 0, hue, m_hist, 1, &hsize, &phranges);
                cv::normalize(m_hist, m_hist, 0, 255, cv::NORM_MINMAX);
                //snprintf(msg, 100, "Hist: %f, %f, %f", (int)m_hist[0], (int)m_hist[1], (int)m_hist[2]);
                //std::cout << m_hist << std::endl;
                
            } else {

                //Tracking
                //cv::add(cv::Scalar::all(100), *image,*m_proc_image[0]);


                //Schritt 4:
                cv::Mat hsv;
                cv::cvtColor(*image, hsv, cv::COLOR_BGR2HSV);

                cv::Mat hue;
                int ch[] = {0, 0};
                hue.create(hsv.size(), hsv.depth());
                cv::mixChannels(&hsv, 1, &hue, 1, ch, 1);

                //Schritt 5: Rückprojektion
                cv::Mat backproj;
                float hranges[] = {0, 180};
                const float* phranges = hranges;
                cv::calcBackProject(&hue, 1, 0, m_hist, backproj, &phranges);

                //Schritt 6:
                cv::Mat mask;
                cv::inRange(hsv, cv::Scalar(0, smin, MIN(vmin, vmax)),
                        cv::Scalar(180, 256, MAX(vmin, vmax)), mask);

                backproj &= mask;
                //ZaK: good pratice to use debug output :-)
                *m_proc_image[2] = backproj;

                //Schritt 7: finale
                cv::RotatedRect trackBox = cv::CamShift(backproj, m_trackWindow,
                        cv::TermCriteria(cv::TermCriteria::EPS |
                        cv::TermCriteria::COUNT, 10, 1));

                cv::ellipse(*m_proc_image[0], trackBox, cv::Scalar(0, 0, 255), 3, cv::LINE_AA);

            }
    } else {
        std::cout << "Bitte auf Farbbild umstellen " << std::endl;
    }
        
        std::cout << "counter: " << m_counter << ", vmin: " << vmin << ", vmax: " << vmax << ", smin: " << smin << std::endl; 

        m_counter++;

        return (SUCCESS);
}









