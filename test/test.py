from lemmagen3 import Lemmatizer

print(Lemmatizer.list_supported_languages())

a = Lemmatizer('en')
word = 'cats'
print('{}->{}'.format(word, a.lemmatize(word)))

b = Lemmatizer('sl')
word = 'ljudje'
print('{}->{}'.format(word, b.lemmatize(word)))
