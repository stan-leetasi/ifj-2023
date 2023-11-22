// Nevyuzivany parameter, identifikator parametru je _

// Hlavni telo programu
write("Zadejte cislo pro vypocet faktorialu: ")
let inp = readInt()

// pomocna funkce pro dekrementaci celeho cisla o zadane cislo
func decrement(of n: Int, unused_var _: Int, by m: Int) -> Int {
	return n - m 
}

// Definice funkce pro vypocet hodnoty faktorialu
func factorial(_ n : Int) -> Int {
	var result : Int?
	if (n < 2) {
		result = 1
	} else {
		var inp_unnilled = inp!
		let decremented_n = decrement(of: n, unused_var: inp_unnilled, by: 1)
		let temp_result = factorial(decremented_n)
		result = n * temp_result
	}
	return result!
}

// pokracovani hlavniho tela programu
if let inp {
	if (inp < 0)	{ // Pokracovani hlavniho tela programu
		write("Faktorial nelze spocitat!")
	} else {
		let vysl = factorial(inp)
		write("Vysledek je: ", vysl)
	}
} else {
	write("Chyba pri nacitani celeho cisla!")
}
