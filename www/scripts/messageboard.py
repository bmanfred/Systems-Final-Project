#!/usr/bin/env python3

import cgi
file_b = 'contents.txt'

print('HTTP1/.0 200 OK')
print('Content-Type: text/html')
print()

def add_to_book(name, message):
    with open(file_b, 'a') as book:
        book.write(f'<p>{name} says: {message}</p><br>')
    book.close()

form = cgi.FieldStorage()
print('<h1>Add Your Message to the Message Board!</h1>')
print('''
<form action>
    Screen Name: <input type='text' name='identity'><br>
    Message:     <input type='text' name='message'><br>
    <input type='submit' value='Add to Message Board'>
</form>
''')


if 'identity' and 'message' in form:
    name = form.getvalue('identity')
    message = form.getvalue('message')
    add_to_book(name, message);

f = open(file_b, 'r')
for line in f.readlines():
    print(line)
f.close()






 
