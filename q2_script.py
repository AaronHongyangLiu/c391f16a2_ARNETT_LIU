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


rectangles = [  # [x1,y1,x2,y2]
                [2, 25, 5, 23], [3, 20, 7, 17], [1, 15, 4, 13], [1, 3, 4, 0],
                [6, 24, 9, 21], [7, 20, 9, 15], [6, 8, 13, 3], [17, 22, 20, 9],
                [19, 12, 24, 9], [19, 8, 23, 6], [21, 25, 26, 21],
                [20, 17, 30, 15], [25, 16, 28, 12], [13, 22, 17, 19]
]


### TESTING CODE ###
test_node = [0, 1, 1, 0]
test_box = [1, 2, 2, 1]

change = calculate_increasing_area(original_box=test_node, new_box=test_box)
print change  # should print 3
