var outerCounter = 1

while outerCounter <= 3 {
    var innerCounter = 1
    
    while innerCounter <= 5 {
        write("Outer: ", outerCounter, ", Inner: ", innerCounter, "\n")
        innerCounter = innerCounter + 1
    }
    
    outerCounter = outerCounter + 1
}
