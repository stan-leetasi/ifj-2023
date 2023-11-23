var x : String = "abcd"
let t=substring(of:x, startingAt:0, endingBefore:5)
if let t {
    write("wrong\n")
}
else {
    write("ok\n")
}

let m = substring(of:"abc", startingAt:0, endingBefore:5)
if let m {
    write("wrong\n")
}
else {
    write("ok\n")
}

let n = substring(of:"abcdefg", startingAt:2, endingBefore:1)
if let n {
    write("wrong\n")
}
else {
    write("ok\n")
}
