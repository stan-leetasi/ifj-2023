func f (_ x : Int)->Int    // seznam parametru
/* deklarace funkce */ {
  if(x<10){return x-1}else{let x = x - 1
    write("calling g with ", x, "\n")
    let res = g(X: x)
    return res
  }
}

func g(X x:Int) -> Int { // povodne func g(x x:Int) -> Int
  if (x > 0) {
    write("calling f with ", x, "\n") 
    let x = f(x) // modifikace parametru x, ale az po zavolani funkce f
    return x 
  } else {
  
  return 200
  
  }
}

let res = g(X:10)
write("res: ", res, "\n")


