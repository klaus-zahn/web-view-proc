
-include .config

OPENCV_PATH=$(CONFIG_OPENCV_PATH)

#opencv includes
OPENCV_INC_host := -I $(OPENCV_PATH)/OpenCV4.1.1/linux-obj_rel/include/opencv4
OPENCV_INC_host += -I $(OPENCV_PATH)/OpenCV4.1.1/linux-obj_rel/include/opencv4/opencv2

OPENCV_INC_target := -I $(OPENCV_PATH)/OpenCV4.1.1/arm-obj_rel/include/opencv4
OPENCV_INC_target += -I $(OPENCV_PATH)/OpenCV4.1.1/arm-obj_rel/include/opencv4/opencv2


#opencv libs
LIBS := -lopencv_dnn -lopencv_gapi -lopencv_highgui -lopencv_ml -lopencv_objdetect -lopencv_photo -lopencv_stitching -lopencv_video -lopencv_calib3d -lopencv_features2d -lopencv_flann -lopencv_videoio -lopencv_imgcodecs -lopencv_imgproc -lopencv_core

#opencv 3rd party libs
LIBS += -littnotify -llibprotobuf -llibjpeg-turbo -llibwebp -llibpng -llibtiff -llibjasper -lIlmImf -lquirc -lzlib

#further dependencies
LIBS += -ldl
LIBS += -lrt
LIBS += -lm -lpthread


OPENCV_LIBS_host := -L$(OPENCV_PATH)/OpenCV4.1.1/linux-obj_rel/lib
OPENCV_LIBS_host += -L$(OPENCV_PATH)/OpenCV4.1.1/linux-obj_rel/lib/opencv4/3rdparty
OPENCV_LIBS_host += $(LIBS)

OPENCV_LIBS_target := -L$(OPENCV_PATH)/OpenCV4.1.1/arm-obj_rel/lib 
OPENCV_LIBS_target += -L$(OPENCV_PATH)/OpenCV4.1.1/arm-obj_rel/lib/opencv4/3rdparty
OPENCV_LIBS_target += $(LIBS) -ltegra_hal