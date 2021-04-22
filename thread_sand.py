#!/usr/bin/env python
# coding: utf-8
"""
Trashcan V 1.0.6
=====================================
"""

# %%
# Parts of this code were reused from the open source Tensorflow guides to run Saved
# and TFlite models using a tkinter GUI 

# %%
# Download the test images
# ~~~~~~~~~~~~~~~~~~~~~~~~
# First we will download the images that we will use throughout this tutorial. The code snippet
# shown bellow will download the test images from the `TensorFlow Model Garden <https://github.com/tensorflow/models/tree/master/research/object_detection/test_images>`_
# and save them inside the ``data/images`` folder.
import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'    # Suppress TensorFlow logging (1)
import pathlib
import tensorflow as tf

tf.get_logger().setLevel('ERROR')           # Suppress TensorFlow logging (2)

# Enable GPU dynamic memory allocation
gpus = tf.config.experimental.list_physical_devices('GPU')
for gpu in gpus:
    tf.config.experimental.set_memory_growth(gpu, True)

def download_images():
    base_url = 'https://raw.githubusercontent.com/tensorflow/models/master/research/object_detection/test_images/'
    filenames = ['image1.jpg']
    image_paths = []
    for filename in filenames:
        image_path = tf.keras.utils.get_file(fname=filename,
                                            origin=base_url + filename,
                                            untar=False)
        image_path = pathlib.Path(image_path)
        image_paths.append(str(image_path))
    return image_paths

CWD = os.getcwd()
#print("The CWD is: "+CWD)
#IMAGE_PATHS = download_images()
IMAGE_PATHS = []

# %%
# Download the model
# ~~~~~~~~~~~~~~~~~~
# The code snippet shown below is used to download the pre-trained object detection model we shall
# use to perform inference. The particular detection algorithm we will use is the
# `CenterNet HourGlass104 1024x1024`. More models can be found in the `TensorFlow 2 Detection Model Zoo <https://github.com/tensorflow/models/blob/master/research/object_detection/g3doc/tf2_detection_zoo.md>`_.
# To use a different model you will need the URL name of the specific model. This can be done as
# follows:
#
# 1. Right click on the `Model name` of the model you would like to use;
# 2. Click on `Copy link address` to copy the download link of the model;
# 3. Paste the link in a text editor of your choice. You should observe a link similar to ``download.tensorflow.org/models/object_detection/tf2/YYYYYYYY/XXXXXXXXX.tar.gz``;
# 4. Copy the ``XXXXXXXXX`` part of the link and use it to replace the value of the ``MODEL_NAME`` variable in the code shown below;
# 5. Copy the ``YYYYYYYY`` part of the link and use it to replace the value of the ``MODEL_DATE`` variable in the code shown below.
#
# For example, the download link for the model used below is: ``download.tensorflow.org/models/object_detection/tf2/20200711/centernet_hg104_1024x1024_coco17_tpu-32.tar.gz``

# Download and extract model
def download_model(model_name, model_date):
    base_url = 'http://download.tensorflow.org/models/object_detection/tf2/'
    model_file = model_name + '.tar.gz'
    model_dir = tf.keras.utils.get_file(fname=model_name,
                                        origin=base_url + model_date + '/' + model_file,
                                        untar=True)
    return str(model_dir)

MODEL_DATE = '20200711'
MODEL_NAME = 'ssd_mobilenet_v1_1_default_1'
LITE_MODEL_NAME = MODEL_NAME + '.tflite'
#PATH_TO_MODEL_DIR = download_model(MODEL_NAME, MODEL_DATE)
PATH_TO_MODEL_DIR = CWD + "/models/" + MODEL_NAME
PATH_TO_LITE_MODEL = CWD + "/tflite_models/" + LITE_MODEL_NAME
RUN_LITE_MODEL = True
# %%
# Download the labels
# ~~~~~~~~~~~~~~~~~~~
# The coode snippet shown below is used to download the labels file (.pbtxt) which contains a list
# of strings used to add the correct label to each detection (e.g. person). Since the pre-trained
# model we will use has been trained on the COCO dataset, we will need to download the labels file
# corresponding to this dataset, named ``mscoco_label_map.pbtxt``. A full list of the labels files
# included in the TensorFlow Models Garden can be found `here <https://github.com/tensorflow/models/tree/master/research/object_detection/data>`__.

# Download labels file



def download_labels(filename):
    base_url = 'https://raw.githubusercontent.com/tensorflow/models/master/research/object_detection/data/'
    label_dir = tf.keras.utils.get_file(fname=filename,
                                        origin=base_url + filename,
                                        untar=False)
    label_dir = pathlib.Path(label_dir)
    return str(label_dir)

LABEL_FILENAME = 'mscoco_label_map.pbtxt'

#PATH_TO_LABELS = download_labels(LABEL_FILENAME)
PATH_TO_LABELS = CWD+"/labels/"+ LABEL_FILENAME
print("Model: "+PATH_TO_MODEL_DIR)

# %%
# Load the model
# ~~~~~~~~~~~~~~
# Next we load the downloaded model
import time
from object_detection.utils import label_map_util
from object_detection.utils import visualization_utils as viz_utils

PATH_TO_SAVED_MODEL = PATH_TO_MODEL_DIR + "/saved_model"

print('Loading model...', end='')
start_time = time.time()
if RUN_LITE_MODEL:
    interpreter = tf.lite.Interpreter(PATH_TO_LITE_MODEL)
    interpreter.allocate_tensors()
else:

    # Load saved model and build the detection function
    detect_fn = tf.saved_model.load(PATH_TO_SAVED_MODEL)

end_time = time.time()
elapsed_time = end_time - start_time
print('Done! Took {} seconds'.format(elapsed_time))

# %%
# Load label map data (for plotting)
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Label maps correspond index numbers to category names, so that when our convolution network
# predicts `5`, we know that this corresponds to `airplane`.  Here we use internal utility
# functions, but anything that returns a dictionary mapping integers to appropriate string labels
# would be fine.

category_index = label_map_util.create_category_index_from_labelmap(PATH_TO_LABELS,
                                                                    use_display_name=True)


import numpy as np
from PIL import Image
import matplotlib.pyplot as plt
import warnings
warnings.filterwarnings('ignore')   # Suppress Matplotlib warnings
import cv2
import serial

serial_port = serial.Serial(
    port="/dev/ttyTHS1",
    baudrate=115200,
    bytesize=serial.EIGHTBITS,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
)
time.sleep(1)
BOTTLE_NUM = 43
#mess = ""
detect_msg = ""
from tkinter import *
from PIL import ImageTk, Image
import threading
import queue


def GetDetectionMessage():
    result = ""
    print("waiting for detection message!")
    try:

        while True:
            if serial_port.inWaiting()>0:
                data = serial_port.read()
                print(data)
                result = result+str(data)
                if data == b'\0':
                    break
    except Exception as exception_error:
        print("Error occurred in UART communication.")
        print("Error: " + str(exception_error))
    print("message received!")
    print(str(result))
    return result

def SendDetectionMessage(result):
    print("sending detection")
    try:
        if result == "u":
            serial_port.write(b"`u9")
        else: 
            serial_port.write(b"`n9")
        print("message sent")
    except Exception as exception_error:
        print("Error occurred in UART communication.")
        print("Error: " + str(exception_error))
        
    return
    
def GetCameraImg():
    IMAGE_PATHS.clear()
    print("capturing image")
    vid = cv2.VideoCapture(0)
    count = 0
    ret, frame = vid.read()
    cv2.imwrite("eval_img/eval.jpg",frame)
    print("saving image")
    #cv2.imshow('frame',frame)
    IMAGE_PATHS.append(CWD+"/eval_img/eval.jpg")
    return 
     

def load_image_into_numpy_array(path):
    """Load an image from file into a numpy array.

    Puts image into numpy array to feed into tensorflow graph.
    Note that by convention we put it into a numpy array with shape
    (height, width, channels), where channels=3 for RGB.

    Args:
      path: the file path to the image

    Returns:
      uint8 numpy array with shape (img_height, img_width, 3)
    """

    if RUN_LITE_MODEL:
        img = cv2.imread(path)
        img_rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        img_rgb = cv2.resize(img_rgb, (300, 300), cv2.INTER_AREA)
        img_rgb = img_rgb.reshape([1, 300, 300, 3])
        image = img_rgb

    else:
        image = np.array(Image.open(path))
    return image


class GUI:
    def __init__(self, master):
        self.master = master
        self.master.title("Trashcan 1.0")
        self.detection_img = ImageTk.PhotoImage(Image.open(CWD+"/eval_img/eval.jpg"))
        self.butt = Button(self.master,text = "Start Detector", command = lambda: self.start_detect_thread())
        self.static_label = Label(self.master,text = "Current Status: ", font = ('Arial' , 16))
        self.detect_label = Label (self.master, image = self.detection_img)
        self.butt.grid(row = 1, column = 0,sticky = W)
        self.static_label.grid(row = 0, column = 0,sticky = W)
        self.detect_label.grid(row =1, column = 1, sticky = W)
        self.status_label = Label (self.master, text = "Start Detector to Begin", font = ('Arial' , 25))
        self.status_label.grid(row = 0, column = 1, sticky = N)
        self.result_static_label = Label(self.master,text = "Item Detected: ", font = ('Arial' , 16))
        self.result_static_label.grid(row = 2, column = 0)
        self.result_final_label = Label(self.master,text = "", font = ('Arial' , 21))
        self.result_final_label.grid(row = 2,column = 1)
        return
    def update_eval_img(self):
        #displayImage = PhotoImage(file = IMG_PATH)
        self.detection_img = ImageTk.PhotoImage(Image.open(CWD+"/eval_img/eval.jpg"))
        self.detect_label.configure(image = self.detection_img)
        return

    def update_status_label(self,status_message):
        self.status_label.config(text = status_message)
        return

    def update_final_label(self,status_message):
        self.result_final_label.config(text = status_message)
        return
    def process_queue(self):
        try:
            msg = self.queue.get(0)
            # Show result of the task if needed
            self.butt["state"] = "normal"
            
        except queue.Empty:
            self.master.after(100, self.process_queue)
        return

    def start_detect_thread(self):
        self.butt["state"] = "disabled"
        self.update_status_label("Detector Started")
        time.sleep(1)
        self.queue = queue.Queue()
        ThreadedTask(self.queue).start()
        self.master.after(100, self.process_queue)   
        return


    def start(self):
        self.master.mainloop() 
        return


class ThreadedTask(threading.Thread):
    def __init__(self, queue):
        threading.Thread.__init__(self)
        self.queue = queue
    def run(self):
        while True:
            print("starting detection")
            trash_gui.update_status_label("Insert Item to Begin")
            GetDetectionMessage()
            trash_gui.update_status_label("Detection in progress")
            GetCameraImg()

            for image_path in IMAGE_PATHS:
                
                print('Running inference for {}... '.format(image_path), end='')

                image_np = load_image_into_numpy_array(image_path)

                # Things to try:
                # Flip horizontally
                # image_np = np.fliplr(image_np).copy()

                # Convert image to grayscale
                # image_np = np.tile(
                #     np.mean(image_np, 2, keepdims=True), (1, 1, 3)).astype(np.uint8)

                if RUN_LITE_MODEL:
                    input_details = interpreter.get_input_details()
                    output_details = interpreter.get_output_details()

                    input_data = image_np
                    print(input_details)
                    print(output_details)
                    interpreter.set_tensor(input_details[0]['index'], input_data)
                    interpreter.invoke()
                    print(interpreter.get_tensor(output_details[0]['index']))
                    output_dict = {
                                    'detection_boxes' : interpreter.get_tensor(output_details[0]['index']),
                                    'detection_classes' : interpreter.get_tensor(output_details[1]['index']),
                                    'detection_scores' : interpreter.get_tensor(output_details[2]['index']),
                                    'num_detections' : interpreter.get_tensor(output_details[3]['index'])
                                    }

                    output_dict['detection_classes'] = output_dict['detection_classes'].astype(np.int64)
                    print(output_dict['detection_classes'])
                    print(output_dict['detection_scores'])
                    image_np_with_detections = image_np.copy()
                    if BOTTLE_NUM in output_dict['detection_classes'][0]:
                        print("bottle detected")
                        SendDetectionMessage("u")
                    else:
                        SendDetectionMessage("n") 
                    final_detection = GetDetectionMessage()
                    print(final_detection)

                    if str(b'p') in final_detection:
                        trash_gui.update_final_label("Plastic Bottle")
                    elif str(b'g') in final_detection:
                        trash_gui.update_final_label("Glass Bottle")
                    elif str(b'a') in final_detection:
                        trash_gui.update_final_label("Can")
                    elif str(b'n') in final_detection:
                        trash_gui.update_final_label("Trash")
                    trash_gui.update_eval_img()
                #this else statement is incomplete as the detector is only made to run on the Jetson Nano
                #and saved models were found to be extremely slow at runtime. 
                else:
                
                    # The input needs to be a tensor, convert it using `tf.convert_to_tensor`.
                    input_tensor = tf.convert_to_tensor(image_np)
                    # The model expects a batch of images, so add an axis with `tf.newaxis`.
                    input_tensor = input_tensor[tf.newaxis, ...]

                    # input_tensor = np.expand_dims(image_np, 0)
                    detections = detect_fn(input_tensor)

                    # All outputs are batches tensors.
                    # Convert to numpy arrays, and take index [0] to remove the batch dimension.
                    # We're only interested in the first num_detections.
                    num_detections = int(detections.pop('num_detections'))
                    detections = {key: value[0, :num_detections].numpy()
                                for key, value in detections.items()}
                    detections['num_detections'] = num_detections
                    # detection_classes should be ints.
                    detections['detection_classes'] = detections['detection_classes'].astype(np.int64)
                    #print(detections['detection_classes'])
                    image_np_with_detections = image_np.copy()

                    viz_utils.visualize_boxes_and_labels_on_image_array(
                        image_np_with_detections,
                        detections['detection_boxes'],
                        detections['detection_classes'],
                        detections['detection_scores'],
                        category_index,
                        use_normalized_coordinates=True,
                        max_boxes_to_draw=200,
                        min_score_thresh=.30,
                        agnostic_mode=False)
            

if __name__ == "__main__":

    master = Tk()
    trash_gui = GUI(master)
    trash_gui.start()
        #main()
    
        #plt.figure()
        #plt.imshow(image_np_with_detections)

    #im = Image.fromarray(image_np_with_detections)
    #im.save("image1_detected.jpg")
    #print('Done')
#plt.show()

# sphinx_gallery_thumbnail_number = 2
