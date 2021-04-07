from tkinter import *
from PIL import ImageTk, Image
import os
IMG_PATH = os.getcwd()+"/eval_img/eval.jpg"
img = 0
class GUI:
    def __init__(self, master):
        self.master = master
        self.master.title("Trashcan 1.0")
        self.detection_img = ImageTk.PhotoImage(Image.open(IMG_PATH))


    def button():
        panel = Label(master,image = self.detection_img)
        panel.grid(row = 0, column = 1,sticky = W)
        print("button pressed")


def main():
    master = Tk()
    master.title("Trashcan 1.0")
    img = ImageTk.PhotoImage(Image.open(IMG_PATH))
    #frame = Frame(master, height = 1000,width = 1000)
    #frame.pack()
    #panel = Label(master,image = img)
    butt = Button(master,text = "sddsf", command = lambda: button())
    #panel.pack(side = RIGHT)
    #butt.pack(side = LEFT)
    #panel.grid(row = 0, column = 1,sticky = W)
    butt.grid(row = 0, column = 0,sticky = W)
    master.mainloop()


if __name__ == "__main__":
    main()
