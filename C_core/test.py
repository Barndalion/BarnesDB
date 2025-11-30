li = [1,3,5,6]
tar = 4

def bin(li,tar):
    strt = 0
    end = len(li)-1
    
    while strt <= end:
        middle = (end + strt)//2
        if tar == li[middle]:
            return middle
            
        elif tar > li[middle]:
            strt = middle

        else:
            end = middle 

    return strt
        
print(bin(li,tar))