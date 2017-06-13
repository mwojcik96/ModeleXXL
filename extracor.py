import sys

goodout = True
hallError = False
hotelError = False
firstClkError = -1
with open(sys.argv[1], 'r') as f:
    unsorted_list = [line.split() for line in f]
cutted_list = []
for id, rec in enumerate(unsorted_list):
    if(len(rec) > 9):
        if rec[0].isnumeric() and rec[1] == "," and rec[2] == "PID:" and rec[3].isnumeric():
            cutted_list.append(rec)
        else:
            print("line no.", id, "caption:", rec)
            if "ZAJMUJE" in rec:
                goodout = False
            if "ZWALNIAM" in rec:
                goodout = False

sorted_list = sorted(cutted_list, key = lambda x: int(x[0]))
city = int(sys.argv[2])
hall = int(sys.argv[3])
hotel = int(sys.argv[4])
if int(sys.argv[5]): #1 - szczegolowy raporcik
    hallVec = []
    cityVec = [-1] * hall
    for i in range(city):
        hallVec.append(cityVec)
        cityVec = cityVec.copy()
    #hotelVec = [-1] * city
    hotelVec = []
    for i in range(city):
        hotelVec.append([])
    print("0: ",hotelVec, hallVec)
    for ident, w in enumerate(sorted_list):
        try:
            if w[9] == "ZAJMUJE" or w[9] == "ZWALNIAM":
                if w[10] == "SALE":
                    if w[9] == "ZAJMUJE":
                        if hallVec[int(w[14])][int(w[11])] == -1:
                            hallVec[int(w[14])][int(w[11])] = int(w[3])
                        else:
                            if firstClkError == -1:
                                hallError = True
                                firstClkError = w[0]
                    else:
                        hallVec[int(w[14])][int(w[11])] = -1
                else:
                    if w[9] == "ZAJMUJE":
                        hotelVec[int(w[13])].append(int(w[3]))
                        if len(hotelVec[int(w[13])]) > hotel and firstClkError == -1:
                            hotelError = True
                            firstClkError = w[0]
                    else:
                        hotelVec[int(w[13])].remove(int(w[3]))
                print(str(w[0])+":",hotelVec, hallVec)
        except:
            print("error on line", w)
            goodout = False
else:
    hallVec = []
    cityVec = [0] * hall
    for i in range(city):
        hallVec.append(cityVec)
        cityVec = cityVec.copy()
    #hotelVec = [-1] * city
    hotelVec = []
    for i in range(city):
        hotelVec.append(0)
    print("0: ",hotelVec, hallVec)
    for ident, w in enumerate(sorted_list):
        try:
            if w[9] == "ZAJMUJE" or w[9] == "ZWALNIAM":
                if w[10] == "SALE":
                    if w[9] == "ZAJMUJE":
                        hallVec[int(w[14])][int(w[11])] += 1
                        if hallVec[int(w[14])][int(w[11])] > 1 and firstClkError == -1:
                            hallError = True
                            firstClkError = w[0]
                    else:
                        hallVec[int(w[14])][int(w[11])] -= 1
                else:
                    if w[9] == "ZAJMUJE":
                        hotelVec[int(w[13])] += 1
                        if hotelVec[int(w[13])] > hotel and firstClkError == -1:
                            hallError = True
                            firstClkError = w[0]
                    else:
                        hotelVec[int(w[13])] -= 1
                print(str(w[0])+":",hotelVec, hallVec)
        except:
            print("error on line", w)
            goodout = False
if not(goodout):
    print("PROBABLY BAD OUTPUT, CHECK LOG")
if hallError:
    print("hallError, on clock no.", firstClkError)
if hotelError:
    print("hotelError, on clock no.", firstClkError)
