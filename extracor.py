with open('log', 'r') as f:
    unsorted_list = [line.split() for line in f]
sorted_list = sorted(unsorted_list, key = lambda x: int(x[0]))
city = 2
hall = 2
hotel = 5
hallVec = []
cityVec = [0] * hall
for i in range(hall):
    hallVec.append(cityVec)
    cityVec = cityVec.copy()
hotelVec = [0] * city
print(hotelVec, hallVec)
for w in sorted_list:
    if w[9] == "ZAJMUJĘ" or w[9] == "ZWALNIAM":
        if w[10] == "SALĘ":
            if w[9] == "ZAJMUJĘ":
                hallVec[int(w[14])][int(w[11])] += 1
            else:
                hallVec[int(w[14])][int(w[11])] -= 1
        else:
            if w[9] == "ZAJMUJĘ":
                hotelVec[int(w[13])] += 1
            else:
                hotelVec[int(w[13])] -= 1
        print(hotelVec, hallVec)