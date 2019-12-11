class Lemmatizer:
    '''
    This is a thin convenience wrapper aroung RdrLemmatizer which allows to use
    language codes for loading lemmatizer models instead of model file names.
    '''
    def __init__(self, lang=None):
        # import here to avoid namespace pollution
        from . _lemmagen import RdrLemmatizer
        self.__lemmatizer = RdrLemmatizer()
        if lang is not None:
            self.load_language(lang)

    def lemmatize(self, word):
        '''
        Lemmatizes given word.

        Parameters
        ----------
        word : str
            Input word.

        Returns
        -------
        str
            Lemmatized form of the input word.
        '''
        return self.__lemmatizer.lemmatize(word)

    def load_model(self, model):
        '''
        Loads a binary model.

        Parameters
        ----------
        model : str
            Path to the model file.
        '''
        self.__lemmatizer.load_model(model)

    def load_language(self, lang):
        '''
        Loads a model for a given supported language.

        Parameters
        ----------
        lang : str
            Language code in ISO 639-1 (two-letter code).
        '''
        from os.path import join, dirname, abspath, exists
        mpath = join(dirname(abspath(__file__)), 'models', '{}.bin'.format(lang))
        if not exists(mpath):
            raise ValueError('Language "{}" is not supported'.format(lang))
        self.__lemmatizer.load_model(mpath)

    @staticmethod
    def list_supported_languages():
        '''
        Returns a list of supported languages (names of present model files).
        '''
        from os.path import join, dirname, abspath
        from os import listdir
        return sorted([mfname.replace('.bin', '') for mfname in listdir(join(dirname(abspath(__file__)), 'models'))])
