var index = 0
var string = "Swift ifj 2023!"
var max = length(string)
while index < max {
    var substring = substring(of: string, startingAt: index, endingBefore: max)
    var test = substring ?? "Invalid substring"
    write("Substring: ", substring)
    index = index + 1
}