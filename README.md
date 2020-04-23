## About

`lemmagen3` is a Python2/3 wrapper for the [Lemmagen lemmatizer](http://lemmatise.ijs.si/).

It is different from other Lemmagen wrappers like [this one on PyPi](https://pypi.org/project/Lemmagen/) because it offers a clean, fast OO interface built with the excellent [pybind11 library](https://github.com/pybind/pybind11) and supports an additional language (Croatian). The models for Slovene and Serbian were also updated. In total, 19 languages are supported:

1.  Bulgarian: `bg`
2.  Croatian: `hr`
3.  Czech: `cs`
4.  English: `en`
5.  Estonian: `et`
6.  Farsi/Persian: `fa`      
7.  French: `fr`  
8.  German: `de`
9.  Hungarian: `hu`
10. Italian: `it`
11. Macedonian: `mk`        
12. Polish: `pl`     
13. Romanian: `ro`            
14. Russian: `ru`             
15. Serbian: `sr`
16. Slovak: `sk`
17. Slovene: `sl`          
18. Spanish: `es`
19. Ukrainian: `uk`


## Installation and requirements


```sh
pip install lemmagen3
```
will install the module for `python 2.7+` and `python 3.5+`.


If you want to build it from source, you will need `pybind11` and a C++ compiler.
Then run

```sh
python setup.py install
```
which will compile and install the module.

## How to use

The following snippet illustrates how to use `lemmagen3`.

```python
from lemmagen3 import Lemmatizer

# first, list all supported languages
print(Lemmatizer.list_supported_languages())

# then, create few lemmatizer objects using ISO 639-1 language codes
# (English, Slovene and Russian)

lem_en = Lemmatizer('en')
lem_sl = Lemmatizer('sl')
lem_ru = Lemmatizer('ru')

# now lemmatize the word "cats" in all three languages
print(lem_en.lemmatize('cats'))
print(lem_sl.lemmatize('mačke'))
print(lem_ru.lemmatize('коты'))

# you can also change the language for an existing Lemmatizer object
# lem_en will now become a French lemmatizer:
lem_en.load_language('fr')

# finally, you can also load your own Lemmagen model
my_lem = Lemmatizer()
my_lem.load_model('/path/to/my/model')
```

Note that the function `lemmatize` accepts single string tokens and **does not split the input string**! If you want to lemmatize a chunk of text you will have to tokenize it first, e.g.:

```python
sentence = 'cats hate dogs'
tokens = sentence.split()
sentence_lemmatized = ' '.join([lem_en.lemmatize(token) for token in tokens])
```

Note also that `lemmagen3` operates on unicode encoded strings so if you use python 2 make sure that your input string is encoded as unicode.


## License

Please note that this repository contains code and binary models compiled and built from different sources which are under different licenses:

1.  C++ files and headers come from [Lemmagen](http://lemmatise.ijs.si/) and are modified and adapted to work as a Python module (**LGPL**)
2.  Binary models are built from Multext and [Multext-east](http://nl.ijs.si/ME/V4/) sources:
    *   Language resources used to build Farsi/Persian, Macedonian, Polish, and Russian models are for **non-commercial use only**.
    *   Language resource for other supported languages are released under **CC BY-SA 4.0**.

The rest of the code in this repository was created by the author and is licensed under the **MIT license**.


## Authors

-   `lemmagen3` is developed by Vid Podpečan (vid.podpecan@ijs.si).
-   The Lemmagen lemmatizer was developed by [Matjaž Juršič](http://lemmatise.ijs.si/Home/Contact).
