import pickle

with open("ai-imu-dr-master/data/2011_09_26_drive_0009_extract.p", 'rb') as file:
    loadf = pickle.load(file)

print(loadf)
