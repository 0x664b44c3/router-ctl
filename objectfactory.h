#ifndef OBJFACTORY_H
#define OBJFACTORY_H

#include <QString>
#include <QObject>
#include <QMap>
#include <variant>

template <class B>
class ObjectFactory;

template <class B> class AbstractFactory
{
public:
    explicit AbstractFactory() {}
    ~AbstractFactory() {}

    virtual B* get(QString, QObject*)=0;
};

template <class B, class T> class FactoryImpl : public AbstractFactory<B>
{
public:
    explicit FactoryImpl() : AbstractFactory<B>() {}
    ~FactoryImpl() {}

    B* get(QString s, QObject* p) final override
    {
        return new T(s, p);
    }

    static bool registerToFactory(ObjectFactory<B> * f, QString id)
    {
        return f->registerClass(id, new FactoryImpl<B, T>());
    }
};

template <class B>
class ObjectFactory
{
public:
    explicit ObjectFactory() {}
    ~ObjectFactory() {
        for(auto & t: mCreators)
        {
            if (t)
                delete t;
        }
    }
    template<class... Args>
    B* create(QString name, QString s, QObject *p) {
        if (!mCreators.contains(name))
            return nullptr;
        if (!mCreators.value(name))
            return nullptr;
        return mCreators[name]->get(s, p);
    }

    bool registerClass(QString name, AbstractFactory<B> * creator)
    {
        if (mCreators.contains(name))
            return false;
        mCreators.insert(name, creator);
        return true;
    }

private:
    QMap<QString, AbstractFactory<B>* > mCreators;
};


#endif
