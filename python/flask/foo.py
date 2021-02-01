from firebase import firebase
import json
countor = 198
COUNTOR_MAX = 200
countor_flag = False
next_flag = True
count = 0
firebase = firebase.FirebaseApplication("xxxxxxxxxxURLxxxxxx", None)

result = firebase.get('Accelerometer', '')
json_object = json.dumps(result, indent = 4)   

js = json.loads(json_object)

def deleteData(id):
    firebase.delete('Accelerometer', id)
    print("\n Deleted ", id)

for key, value in js.items():
    print(value['Count'])
    if(int(value['Count']) == 198 or countor_flag):
        countor_flag = True
        countor = int(value['Count'])
        if(int(value['Count']) == 0):
            countor_flag = False
        if(countor_flag and next_flag):
            print(value)
    if(int(value['Count']) == 0):
        countor_flag = False
        count = count + 1
        print("\n", count, "Series completed\n\n")
        next_flag = False
        

