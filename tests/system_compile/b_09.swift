var outerCountdown = 2

while outerCountdown > 0 {
    write("Outer countdown: ", "\t", outerCountdown, "\n")

    var innerCountdown = 3

    while innerCountdown > 0 {
        if innerCountdown - 2 < 0 {
            write("Inner countdown is negative: ", innerCountdown, "\n")
        } else {
            write("Inner countdown is positive: ", innerCountdown, "\n")
        }

        innerCountdown = innerCountdown - 1
    }

    outerCountdown = outerCountdown - 1
}
