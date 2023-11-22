let stringExample = "Swift ifj 2023!"

let stringLength = length(stringExample)
write("String Length: ", stringLength)

let subString = substring(of: stringExample, startingAt: 0, endingBefore: 5)
var test = subString ?? "Invalid substring"
write("Substring: ", test)
