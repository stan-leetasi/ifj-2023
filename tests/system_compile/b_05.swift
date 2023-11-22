func getUsername() -> String? {
    let isLoggedIn = false

    return isLoggedIn ? "Boris" : nil
}

let username = getUsername() ?? "Michael"
write("Welcome, \(username)!")
