#!/usr/bin/env python3

import cgi

print('HTTP1/.0 200 OK')
print('Content-Type: text/html')
print()
print('<h1>Select your favorite class</h1>');

form = cgi.FieldStorage()





print('''
<form action>
    <input type='radio' name='class' value='Discrete Math'/> Discrete Math
    <input type='radio' name='class' value='Fund Comp'/> Fund Comp
    <input type='radio' name='class' value='Systems'/> Systems
    <input type='radio' name='class' value='Data Structues'/> Data Structures
    <input type='radio' name='class' value='Logic Design'/> Logic Design
    <input type='radio' name='class' value='Compilers'/> Compilers
    <input type='radio' name='class' value='OS'/> OS
    <input type='submit' value='Confirm'/>
</form>
''')

if form.getvalue('class'):
    subject = form.getvalue('class')

if subject == 'Compilers':
    print('<h1>You are a sweat</h1>')
elif subject == 'Logic Design':
    print('<h1>You are a sweat</h1>')
else:
    print('<h1>Congrats, you are not a sweat</h1>')

