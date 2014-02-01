
### damn scikit-learn ###
import numpy as np

class discrete_nb_clf(object):
    def __init__(self, alpha=0.01):
        self.alpha_ = alpha

    def fit(self, X, Y, feature_categories=None):
        # not idiomatic at all-- my bad

        self.n_classes_ = int(Y.max()) + 1
        self.prior_ = np.zeros(self.n_classes_)
        for y in Y:
            self.prior_[y] += 1

        self.n_features_ = X.shape[1]
        if feature_categories is None:
            feature_categories = X.max(axis=0) + 1
        else:
            feature_categories = np.vstack((X.max(axis=0) + 1, feature_categories)).max(axis=0)
        def mk(sz):
            r = np.zeros(sz)
            r.fill(self.alpha_) # laplace smooth this shit
            return r
        self.posterior_ = [
            [mk(feature_categories[i]) for i in xrange(self.n_features_)]
            for y in xrange(self.n_classes_)]

        for x, y in zip(X, Y):
            for i, f in enumerate(x):
                self.posterior_[y][i][f] += 1

        for y in xrange(self.n_classes_):
            for i in xrange(self.n_features_):
                self.posterior_[y][i] /= (self.prior_[y] + self.alpha_ * feature_categories[i])
                self.posterior_[y][i] = np.log(self.posterior_[y][i])

        self.prior_ /= self.prior_.sum()
        self.prior_ = np.log(self.prior_)

    def predict(self, X):
        Y = np.zeros(X.shape[0])
        for idx, x in enumerate(X):
            #for i, f in enumerate(x):
            #    for y in xrange(self.n_classes_):
            #        assert len(self.posterior_[y]) >= len(x)
            #        assert f < len(self.posterior_[y][i])
            xs = [
              (self.prior_[y] + np.sum(self.posterior_[y][i][f] for i, f in enumerate(x)))
              for y in xrange(self.n_classes_)]
            Y[idx] = np.argmax(xs)
        return Y

    def tojson(self):
        import json
        return json.dumps({'prior':self.prior_.tolist(), 'conditionals':[[x.tolist() for x in y] for y in self.posterior_]})

if __name__ == '__main__':

    clf = discrete_nb_clf()

    X = np.array([
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [1, 1, 1],
        [1, 1, 1],
        [1, 1, 1],
    ])

    Y = np.array([0, 0, 0, 1, 1, 1])

    clf.fit(X, Y)
    print clf.prior_
    print clf.predict(X)

    with open('out.json', 'w') as fp:
        print >>fp, clf.tojson()
