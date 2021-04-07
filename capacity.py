totalCap = 14871
avgBottle = 591
avgCan = 473
avgGlass = 405
avgTrash = 591
counter1 = 0
counter2 = 0
counter3 = 0
counter4 = 0

msg = "`a"

if(msg == "`p"):
    while(counter1 < totalCap):
        if(counter1 > totalCap):
            print("Max Capacity Reached For: Bottles")
            break
        else:
            counter1 = counter1 + avgBottle
            print("Capacity Counter Increased For: Bottles by "+str(counter1))
	    
elif(msg == "`a"):
    while(counter2 < totalCap):
        if(counter2 > totalCap):
		    print("Max Capacity Reached For: Cans")
            break
        else:
    	    counter2 = counter2 + avgCan
    	    print("Capacity Counter Increased For: Cans by "+str(counter2))
	  
elif(msg == "`g"):
    while(counter3 < totalCap):
        if(counter3 > totalCap):
            print("Max Capacity Reached For: Glass")
    		break
    	else:
            counter3 = counter3 + avgGlass
    	    print("Capacity Counter Increased For: Glass by "+str(counter3))
	    
else:
    while(counter4 < totalCap):
        if(counter4 > totalCap):
    		print("Max Capacity Reached For: Trash")
    	    break
      	else:
    	    counter4 = counter4 + avgTrash
    	    print("Capacity Counter Increased For: Trash by "+str(counter4))
    
	
