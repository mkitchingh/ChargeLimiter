"""Exercise the daemon's actual SQL templates against accessory identifiers."""
import ctypes
import ctypes.util
import json
from pathlib import Path
import re
import sqlite3

source = (Path(__file__).parents[1] / 'ChargeLimiter/daemon.mm').read_text()
templates = [json.loads('"' + x + '"') for x in re.findall(
    r'sqlite3_mprintf\("((?:\\.|[^"\\])*)"', source)]
create = next(x for x in templates if x.startswith('create table'))
insert = next(x for x in templates if x.startswith('insert'))
select = next(x for x in templates if x.startswith('select'))
library = ctypes.CDLL(ctypes.util.find_library('sqlite3'))
library.sqlite3_mprintf.restype = ctypes.c_void_p
library.sqlite3_mprintf.argtypes = [ctypes.c_char_p]
library.sqlite3_free.argtypes = [ctypes.c_void_p]


def sql(template, identifier, *numbers):
    ptr = library.sqlite3_mprintf(template.encode(), identifier.encode(),
                                 *(ctypes.c_int(n) for n in numbers))
    assert ptr
    try:
        return ctypes.string_at(ptr).decode()
    finally:
        library.sqlite3_free(ptr)


db = sqlite3.connect(':memory:')
for identifier in ['min5', 'ABC123.min5', 'BAT"quote.hour',
                   'X' * 300 + '.day', 'x"; drop table min5;--']:
    db.execute(sql(create, identifier))
    db.execute(sql(insert, identifier), {'1': 1, '2': '{"CurrentCapacity":50}'})
    assert db.execute(sql(select, identifier, 0, 10)).fetchall() == [
        ('{"CurrentCapacity":50}',)]
assert db.execute('select count(*) from min5').fetchone() == (1,)
print('History SQL: normal, dotted, quoted, long and hostile identifiers passed')
