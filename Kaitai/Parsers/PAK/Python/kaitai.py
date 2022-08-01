from pak import Pak

file = Pak.from_file("Script.pak")

for tmp in file.files:
    with open(tmp.name, 'wb') as file:
        file.write(tmp.data)
