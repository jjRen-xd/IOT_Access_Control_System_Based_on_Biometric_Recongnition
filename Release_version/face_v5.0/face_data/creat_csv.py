import sys  
import os.path  
  
 
if __name__ == "__main__":  
 
    BASE_PATH="../face_data"  
 
    SEPARATOR=";"  
 
    fh = open("csv.txt",'w')  
 
    for dirname, dirnames, filenames in os.walk(BASE_PATH):  
        for subdirname in dirnames:  
            subject_path = os.path.join(dirname, subdirname)  
            for filename in os.listdir(subject_path):  
                abs_path = "%s/%s" % (subject_path, filename)  
                print ("%s%s%s" % (abs_path, SEPARATOR, subdirname))  
                fh.write(abs_path)  
                fh.write(SEPARATOR)  
                fh.write(subdirname) #label
                fh.write("\n")   
    print("creat_csv complete")     
    fh.close()  
