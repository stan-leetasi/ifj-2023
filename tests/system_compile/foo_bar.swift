func bar (with param : String) -> String {
    if (param != "") {
        let r : String = foo(param)
        return r
    }
    else {}
    return param
}

func foo(_ par:String) -> String {
    write("foo: ", par, "\u{0a}")
    var l = length(par)
    l = l - 1
    let help = substring(of: par, startingAt:0, endingBefore: l)
    let par = help ?? ""
    let ret = bar(with: par)
    write("foo: ", par, "\u{0a}")
    return ret
}

bar(with: "ABCD")
