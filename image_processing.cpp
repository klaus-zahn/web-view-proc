

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
            m_trackWindow = cv::Rect(310, 220, 50, 50);
                
            cv::Mat hsv;
            cv::cvtColor(*image, hsv, cv::COLOR_BGR2HSV);
                     
            cv::Mat roi = hsv(m_trackWindow);
            cv::Scalar roi_mean = cv::mean(roi);
                       
            char msg[100];
            snprintf(msg, 100, "mean colors: h = %3.1f, s = %3.1f, v = %3.1f", roi_mean[0], roi_mean[1], roi_mean[2]);
            cv::rectangle(*m_proc_image[0], m_trackWindow, cv::Scalar(0,0,255), 3);
            //ZaK: remove obsolete code (or use #if statement e.g. for debug output)
            //cv::subtract(cv::Scalar::all(255), *image, *m_proc_image[0]);
            cv::putText(*m_proc_image[0], msg, cv::Point(10,20), cv::FONT_HERSHEY_PLAIN,1,cv::Scalar(0,0,255));
            //std::cout << msg << std::endl; //Ausgabe auf Konsole
            cv::Mat hue; //mat = matritze
            int ch[] = {0, 0};
            //12
            hue.create(roi.size(), roi.depth()); //die matrize hue auf das rote rechteck festlegen
            cv::mixChannels(&roi, 1, &hue, 1 , ch , 1); //selektiert den farbwert von innerhalb des Bildbereichs des Rechtecks und speichert dies in hue ab
            
            int hsize = 16; //ANzahl unteteilungen
            float hranges[] = {0, 180}; //Fabwert geht von 0 bis 180 --> grenzen
            const float* phranges = hranges;
            
            cv::calcHist(&hue, 1, 0, hue, m_hist, 1, &hsize, &phranges);// Histogram wird generiert und in m_hist abgespeichert
            cv::normalize(m_hist, m_hist, 0, 255, cv::NORM_MINMAX); //HIstogram wird normalisiert --> Wertebereich von 0 bis255
            std::cout << m_hist; //Ausgabe auf Konsole
        } else {

            // Tracking
           cv::Mat hsv;
           cv::cvtColor(*image, hsv, cv::COLOR_BGR2HSV);
           cv::Mat hue;
           
           int ch[] = {0, 0};
           hue.create(hsv.size(), hsv.depth());
           cv::mixChannels(&hsv, 1, &hue, 1, ch, 1); //hue vom ganzen bild herausfinden
           
           cv::Mat backproj;
           float hranges[] = {0, 180};
           const float* phranges = hranges;
           cv::calcBackProject(&hue, 1, 0, m_hist, backproj, &phranges); //gleich wie auto traking funktion bei matlab--> vergleicht den mittelwert von m_hist mit dem Farbhostigramm des livebildes und sucht nach ähnlichkeiten
           //hue--> farbwert der aktuellen pixel
           //m_hist --> ergleichswert 
           cv::Mat mask;
           cv::inRange(hsv, cv::Scalar(0, smin, MIN(vmin, vmax)),
           cv::Scalar(180, 256, MAX(vmin, vmax)), mask);
           backproj &= mask; //korrektur mit treshhold = 180
           
           
           cv::RotatedRect trackBox = cv::CamShift(backproj, m_trackWindow,
           cv::TermCriteria(cv::TermCriteria::EPS |
           cv::TermCriteria::COUNT, 10, 1));
           
           cv::ellipse(*m_proc_image[0], trackBox, cv::Scalar(0, 0, 255), 3,cv::LINE_AA);
           
           cv::add(cv::Scalar::all(100), backproj, *m_proc_image[1]);
        }

        std::cout << "counter: " << m_counter << ", vmin: " << vmin << ", vmax: " << vmax << ", smin: " << smin << std::endl;

        m_counter++;

    } else {
        std::cout << "Bitte auf Farbbild umstellen " << std::endl;
    }

    return (SUCCESS);
}










