/* implicitna /* inicializacia na  */ nil */
let n : Double?
if let n {
    var n : Double = n
    write(n)
}else{
    write(n) // nic nevypise
    write("") // tiez nic nevypise
    var _x1 = n
    write(_x1)  // taktiez nic nevypise
    write("n is nil\n")
}