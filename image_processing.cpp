

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

        int width = 100;
        int height = 100;
        int topLeftx = (640-width)/2;
        int topLefty = (480-height)/2;
        
        
        
        int vmin, vmax, smin;

        vmin = m_valueMin;
        vmax = m_valueMax;
        smin = m_saturationMin;

        if (!image) return (EINVALID_PARAMETER);


        if (!m_startButton) {
            // Initialisierung

            //Nur für Farbbilder Werte berechnen
            //ZaK put this statement outside the if(!m_startButton) to avoid code duplication
            if(image->channels() > 1) {  
                *m_proc_image[0] = image->clone();

               //Rechteck zeichnen
               m_trackWindow = cv::Rect(topLeftx ,topLefty, width, height);
                cv::rectangle( *m_proc_image[0], m_trackWindow, cv::Scalar(0,0,255), 3, cv::LINE_AA);

               //HSV Werte auslesen
               cv::Mat hsv;
               cv::cvtColor(*image, hsv, cv::COLOR_BGR2HSV);
               cv::Mat roi = hsv(m_trackWindow);
            
                cv::Scalar roi_mean = cv::mean(roi);
            
                char msg[100];
                
                //werte ausgeben
                snprintf(msg, 100, "mean values: h = %3.1f, s = %3.1f, v = %3.1f", roi_mean[0], roi_mean[1], roi_mean[2]);
                cv::putText(*m_proc_image[1],msg,cv::Point(10,20),cv::FONT_HERSHEY_PLAIN,1,cv::Scalar(0,0,255));
                std::cout << msg << std::endl;
                
                //rue Werte selektieren
                cv::Mat hue;
                int ch[] = {0, 0};
                hue.create(roi.size(), roi.depth());
                cv::mixChannels(&roi, 1, &hue, 1, ch, 1);  //vom roi Kanal (im Rechteck) werden die hue werte selektiert
            
                //Histogramm auslesen des Farbkanals hue
                int hsize = 16; //Anzahl Histogrammbins
                float hranges[] = {0, 180}; //Farbwert Grenzen im Bild
                const float* phranges = hranges;
                cv::calcHist(&hue, 1, 0, hue, m_hist, 1, &hsize, &phranges); // m_hist ist Resultat
                cv::normalize(m_hist, m_hist, 0, 255, cv::NORM_MINMAX); //normieren auf Parameterwerte von 0 bis 255
            
                //ZaK: use #if statement to allow for quick usage of debug output
                //cv::Scalar hist_mean = cv::mean(m_hist);
                
                //ZaK: rather use a loop for output 
                // Ausgabe der Histogrammbins auf der KOnsole. (Kannte keine alternative Ausgabemöglichkeit) 
                /* snprintf(msg, 100, "%3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f,", 
                         m_hist.at<float>(0),m_hist.at<float>(1),m_hist.at<float>(2),m_hist.at<float>(3),
                         m_hist.at<float>(4),m_hist.at<float>(5),m_hist.at<float>(6),m_hist.at<float>(7),
                         m_hist.at<float>(8),m_hist.at<float>(9),m_hist.at<float>(10),m_hist.at<float>(11),
                         m_hist.at<float>(12),m_hist.at<float>(13),m_hist.at<float>(14),m_hist.at<float>(15));
       
                 cv::putText(*m_proc_image[1],msg,cv::Point(10,20),cv::FONT_HERSHEY_PLAIN,1,cv::Scalar(0,0,255));
                 std::cout << msg << std::endl;
                 */
            
            } else {
                std::cout << "Bitte auf Farbbild umstellen " << std::endl;
            }
             
        } else {
            //Tracking
            if(image->channels() > 1) {
             *m_proc_image[0] = image->clone();
             
              //HSV Werte auslesen
               cv::Mat hsv;
               cv::cvtColor(*image, hsv, cv::COLOR_BGR2HSV);
               
               //Extrahieren des HSV Farbraums hue
               cv::Mat hue; 
               int ch[] = {0, 0}; 
               hue.create(hsv.size(), hsv.depth()); 
               cv::mixChannels(&hsv, 1, &hue, 1, ch, 1);
                
               //Rückprojektion Histogramm auf Farbkanal 
               cv::Mat backproj; //wahrscheinlicheit für Auftreten des Objektes
               float hranges[] = {0, 180}; //Farbraum Grenze 
               const float* phranges = hranges; 
               cv::calcBackProject(&hue, 1, 0, m_hist, backproj, &phranges);
               *m_proc_image[2] = backproj;
               
               //Begrenzung des Farbraums durch die Slider
               cv::Mat mask;
               cv::inRange(hsv, 
                       cv::Scalar(0, smin, MIN(vmin, vmax)), 
                       cv::Scalar(180, 256, MAX(vmin, vmax)), mask);
               backproj &= mask;
               *m_proc_image[1] = backproj;
               
               //optimierung des Tracking Fensters
               cv::RotatedRect trackBox = cv::CamShift(backproj, m_trackWindow, 
                       cv::TermCriteria(cv::TermCriteria::EPS | cv::TermCriteria::COUNT, 10, 1));
               
               //Tracking Resultat in Ausgabebild einzeichnen
               cv::ellipse(*m_proc_image[0], trackBox, cv::Scalar(0, 0, 255), 3, cv::LINE_AA);
                
            } else {
                std::cout << "Bitte auf Farbbild umstellen " << std::endl;
            }
            
        }

        //std::cout << "counter: " << m_counter << ", vmin: " << vmin << ", vmax: " << vmax << ", smin: " << smin << std::endl; 

        m_counter++;

        return (SUCCESS);
}









