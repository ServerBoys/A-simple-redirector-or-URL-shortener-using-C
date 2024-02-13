# A simple redirector / URL shortener using C
This is my attempt on creating a URL shortener using pure C programming language. Note that the program needs you to set up database with data. It's sole purpose is to redirect users to "url" mentioned in the database. (And you also need to tinker a bit in the c file to change your host, port, and such for webserver and database).

```
Note that this is a dummy project I am trying out.
```

I also have a rust version of this project, which is much more mature than this one. Check it out!

## Run

- Step 1: Edit the code with your database host, port, user, password, database name and such. Plus, set it up with host and port to serve it.
- Step 2: Install mysqlclient and microhttpd binaries + headers if you haven't (for Debian, it is `apt install mysqlclient microhttpd`). I am assuming that you have gcc installed and ready to go.
- Step 3: Compile the file using `gcc redirector.c -o redirector --lmysqlclient --lmicrohttpd`.
- Step 4: Voila, there it is, your executable. Run `./redirector` (or `./redirector.exe` in windows)

**Copyright (c) 2024 Author. All Rights Reserved.**
