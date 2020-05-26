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
    
    int height = 60;    //Höhe für das Rechteck in Bildverarbeitung 1
    int width = 60;     //Breite für das Rechteck in Bildverarbeitung 1
    
    char msg[100];
    char msg1[100];

    if (!image) return (EINVALID_PARAMETER);


    if (image->channels() > 1) {    //Überprüfung ob Farbbild
        
        //Image clonen
        *m_proc_image[0] = image->clone();
        
        if (!m_startButton) {
            //Initialisierung
            m_trackWindow = cv::Rect(310, 220, height, width);
            
            //Für die Konvertierung des Farbraums
            cv::Mat hsv;
            cv::cvtColor(*image, hsv, cv::COLOR_BGR2HSV);
                     
            cv::Mat roi = hsv(m_trackWindow);
            cv::Scalar roi_mean = cv::mean(roi);
            
            cv::Rect detect_window = cv::Rect((image->cols-width)/2, (image->rows-height)/2, width, height);
            cv::rectangle(*m_proc_image[0], detect_window, cv::Scalar(0,255,0), 3);
                     
            
            //Schreiben auf die Console
            //snprintf(msg, 100, "mean colors: hue = %3.1f, saturation = %3.1f, value =%3.1f", roi_mean[0], roi_mean[1], roi_mean[2]);
            //std::cout << msg << std::endl;
            
            //Schreiben auf die Bildverarbeitung 1 Ausgabe
            //ZaK: output to image a better solution than on a console :-)
            //only minor problem is the jpg compression of the image by the web server which makes
            //small fonts and certain colors difficult to read
            snprintf(msg, 100, "mean values are: h = %3.1f, s = %3.1f, v = %3.1f", roi_mean[0], roi_mean[1], roi_mean[2]);
            cv::putText(*m_proc_image[0], msg, cv::Point(10,460), cv:: FONT_HERSHEY_PLAIN, 1, cv::Scalar(0,255,0));

            cv::Mat hue;
            int ch[] = {0, 0};
            hue.create(roi.size(), roi.depth());      // Bestimmt die Grösse von hue
            cv::mixChannels(&roi, 1, &hue, 1, ch, 1); // Selektiert den Hue Wert

            int hsize = 16;
            float hranges[] = {0, 180}; //Farbraum in Opencv, darum von 0 bis 180
            const float* phranges = hranges;
            cv::calcHist(&hue, 1, 0, hue, m_hist, 1, &hsize, &phranges); 
            cv::normalize(m_hist, m_hist, 0, 255, cv::NORM_MINMAX);

            //Ausgabe auf der Console
            std::cout << "hist:" << m_hist << std::endl;
            
        } else {
            // Tracking
            
            cv::Mat hsv;
            cv::cvtColor(*image, hsv, cv::COLOR_BGR2HSV);
            cv::Mat hue;
            int ch[] = {0, 0};
            hue.create(hsv.size(), hsv.depth());
            cv::mixChannels(&hsv, 1, &hue, 1, ch, 1);
            
            cv::Mat backproj;
            float hranges[] = {0, 180};     //Farbraum in Opencv, darum von 0 bis 180
            const float* phranges = hranges;
            cv::calcBackProject(&hue, 1, 0, m_hist, backproj, &phranges);
            
            cv::Mat mask;
            cv::inRange(hsv, cv::Scalar(0, smin, MIN(vmin, vmax)), cv::Scalar(180, 256, MAX(vmin, vmax)), mask);
            backproj &= mask;  //Korrektur mit dem Tresholdwert
            
            cv::add(cv::Scalar::all(100), backproj, *m_proc_image[1]);
                      
            cv::RotatedRect trackBox = cv::CamShift(backproj, m_trackWindow, cv::TermCriteria(cv::TermCriteria::EPS | cv::TermCriteria::COUNT, 10, 1));
            cv::ellipse(*m_proc_image[0], trackBox, cv::Scalar(0, 255, 0), 3, cv::LINE_AA);
        }
        
        snprintf(msg1, 100, "counter: %i, vmin: %i, vmax: %i, smin: %i", m_counter, vmin, vmax, smin);  
        cv::putText(*m_proc_image[0], msg1, cv::Point(10,20), cv:: FONT_HERSHEY_PLAIN, 1, cv::Scalar(0,255,0)); //Ausgabe im Bildverarbeitung 1
        std::cout << "counter: " << m_counter << ", vmin: " << vmin << ", vmax: " << vmax << ", smin: " << smin << std::endl; // Ausgabe auf der Console

        m_counter++;        // Erhöhung des counter um 1

    } else {
        std::cout << "Bitte auf Farbbild umstellen " << std::endl;      // Falls nicht auf Farbbild
    }
    return (SUCCESS);
}









