def calculate_increasing_area(original_box, new_box):
    '''
    Function calculates the overall change in area given an original box and a box to engulf
    param: original_box the existing MBR or Node. Dimentions are [x1,y1,x2,y2] - top left, bottom right
    param: new_box is the box to be engulfed by the original_box. Given as [x1,y1,x2,y2] - top left, bottom right
    '''
    original_box_area = calculate_area(original_box)
    result_box = [
        min(original_box[0], new_box[0]),
        max(original_box[1], new_box[1]),
        max(original_box[2], new_box[2]),
        min(original_box[3], new_box[3]),
    ]

    result_box_area = calculate_area(result_box)

    return "Box %s increases %d units" % (str(original_box), abs(result_box_area - original_box_area))


def calculate_area(box):
    return ((box[2] - box[0]) * (box[1] - box[3]))  # width * height




R1 = [2, 25, 5, 23]
R2 =[3, 20, 7, 17]
R3 = [1, 15, 4, 13]
R4 = [1, 3, 4, 0]
R5 = [6, 24, 9, 21]
R6 = [7, 20, 9, 15]
R7 = [6, 8, 13, 3]
R8 = [17, 22, 20, 9]
R9 = [19, 12, 24, 9]
R10 = [19, 8, 23, 6]
R11 = [21, 25, 26, 21]
R12 = [20, 17, 30, 15]
R13 = [25, 16, 28, 12]
R14 = [13, 22, 17, 19]



# original_boxpr = [R3[0],R3[1],R7[2],0] # PURPLE
# original_boxp = [R1[0],R1[1],R5[2],R2[3]] # PINK
# original_boxr = [R6[0],R8[1],R8[2],R8[3]] # RED
# original_boxg = [R9[0],R11[1],R11[2],R10[3]] # GREEN


new_box = R12

change = calculate_increasing_area(original_box=R11, new_box=R13)
print change
