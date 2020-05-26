

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

    if (image->channels() > 1) {
        
        //Image clonen
        
        *m_proc_image[0] = image->clone();
        if (!m_startButton) {
            //Initialisierung
            
            int height =30;
            int width=30;
            cv::Rect detect_window = cv::Rect((image->cols-width)/2, (image->rows-height)/2, width, height);
            m_trackWindow = detect_window;
            
            cv::Mat hsv;
            cv::cvtColor(*image, hsv, cv::COLOR_BGR2HSV);
            
            cv::Mat roi = hsv(m_trackWindow);//hsv(hue) Farbe, Saturation, helligkeit
            cv::Scalar roi_mean = cv::mean(roi);
            
            char msg[100];
            //schreibt in msg hinein
            //cv::subtract(cv::Scalar::all(255), *image, *m_proc_image[0]);
            snprintf(msg, 100, "mean colors: b = %3.1f, g = %3.1f, r = %3.1f", roi_mean[0], roi_mean[1], roi_mean[2]);
            cv::putText(*m_proc_image[0], msg, cv::Point(10,20), cv::FONT_HERSHEY_PLAIN,1,cv::Scalar(0,0,255));
            
            cv::rectangle(*m_proc_image[0], detect_window, cv::Scalar(0,0,255),3,cv::LINE_AA);
            std::cout << msg << std::endl;
            
            cv::Mat hue; int ch[] = {0, 0}; 
            hue.create(roi.size(), roi.depth()); //grösse von hue
            cv::mixChannels(&roi, 1, &hue, 1, ch, 1); //Selektiert den Farbwert
            //inerhalb des bildbereichs und speichert es in hue ab.
            
            int hsize = 16; 
            float hranges[] = {0, 180}; //Farbraum von Rot bis Rot
            const float* phranges = hranges; 
            cv::calcHist(&hue, 1, 0, hue, m_hist, 1, &hsize, &phranges);  //&phranges übergibt pointer pointer
            cv::normalize(m_hist, m_hist, 0, 255, cv::NORM_MINMAX);
            //char[10]=m_hist[];
            //ZaK: for output use for-loop or shortcut:
            //std::cout << m_hist << std::endl;
            std::cout << "hist:" << &m_hist.data[1]<< std::endl;


        } else {
            cv::Mat hsv; 
            cv::cvtColor(*image, hsv, cv::COLOR_BGR2HSV); 
            cv::Mat hue; int ch[] = {0, 0}; 
            hue.create(hsv.size(), hsv.depth()); 
            cv::mixChannels(&hsv, 1, &hue, 1, ch, 1);
            cv::Mat backproj; 
            float hranges[] = {0, 180}; 
            const float* phranges = hranges;
            //&hue vergleichswert (gesamtes Bild) //&phranges range 0-180
            cv::calcBackProject(&hue, 1, 0, m_hist, backproj, &phranges);//Gleich wie das auto ortungprogramm
            cv::Mat mask; 
            cv::inRange(hsv, cv::Scalar(0, smin, MIN(vmin, vmax)), cv::Scalar(180, 256, MAX(vmin, vmax)), mask); //Filtert nur die Werte die über 180 sind sind weiss
            backproj &= mask;
            cv::RotatedRect trackBox = cv::CamShift(backproj, m_trackWindow, cv::TermCriteria(cv::TermCriteria::EPS | cv::TermCriteria::COUNT, 10, 1));
            cv::ellipse(*m_proc_image[0], trackBox, cv::Scalar(0, 0, 255), 3, cv::LINE_AA);
            //ZaK: sth more explicit might be helpful ;-)
            std::cout << "Ich bin die ElSE Schleife" << std::endl;

            cv::add(cv::Scalar::all(100), backproj, *m_proc_image[1]);
        }

        std::cout << "counter: " << m_counter << ", vmin: " << vmin << ", vmax: " << vmax << ", smin: " << smin << std::endl;

        m_counter++;
    }
    //ZaK: a message to switch to color mode could help



        return (SUCCESS);
}









