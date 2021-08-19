## About

`lemmagen3` is a Python 2/3 wrapper for the [Lemmagen lemmatizer](http://lemmatise.ijs.si/) (version [2.2](http://lemmatise.ijs.si/Download/File/Software%23LemmaGen_v2.2.zip)).

It is different from other Lemmagen wrappers like [this one on PyPi](https://pypi.org/project/Lemmagen/) because it offers a clean, fast OO interface built with the excellent [pybind11 library](https://github.com/pybind/pybind11) and supports an additional language (Croatian).

Models for Slovene, Croatian and Serbian are significantly updated and make use of frequency data to prefer most frequent lemmas, e.g., for Slovene: `je->biti` instead of `je->jesti`, `mene->jaz` instead od `mene->mena`, `od->od` instead of `od->oda` etc.

In total, 19 languages are supported:

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

will install the module and language model files. Please note that on `python <=3.5` and `python 2.7` the package will be built from source so you will need a C++ compiler.

**Note:**  If you use `python 3.5.0` or `3.5.1` you will likely get the error shown below. This is a known bug in these two versions so please consider upgrading your Python.

```sh
ImportError: ..._lemmagen.cpython-35m-x86_64-linux-gnu.so: undefined symbol: _PyThreadState_UncheckedGet
```


## How to use

The following snippet illustrates how to use `lemmagen3`.

```python
from lemmagen3 import Lemmatizer

# first, we can list all supported languages
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
