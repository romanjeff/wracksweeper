import threading
import queue
import time

def read_keyboard(inputQueue):
    while True:
        inputStr = input("Gimme some input. ")
        inputQueue.put(inputStr)
        
def main():
    exit_command = "exit"
    
    inputQueue = queue.Queue()
    inputThread = threading.Thread(target=read_keyboard,args=(inputQueue,),
                                   daemon=True)
    inputThread.start()
    
    while True:
        if(inputQueue.qsize() >  0):
            inputStr = inputQueue.get()
            print(format(inputStr))
            if inputStr == exit_command:
                print("Later. ")
                break
        time.sleep(0.01)
    print("end.")
    
if (__name__ == '__main__'):
    main()
