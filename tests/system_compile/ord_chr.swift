var cycles = 5
while (cycles > 0) {
    let A : String = "a"
    var x : String = A
    while (x <= "z") {
        write(x)
        var i = ord(x)
        i = 1 + i
        x = chr(i)
    }
    write("\n")
    cycles = cycles - 1
}
