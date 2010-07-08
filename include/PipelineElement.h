#ifndef PIPELINEELEMENT_H
#define PIPELINEELEMENT_H

#include <map>
#include <stdexcept>
#include <QString>
#include <QObject>
#include <QMetaType>

#include "RefPtr.h"
#include "PlvExceptions.h"

namespace plv
{
    class IInputPin;
    class IOutputPin;
    class Pipeline;

    typedef enum _PlvPipelineElementState
    {
        PLV_PLE_STATE_UNINITIALIZED,
        PLV_PLE_STATE_NOT_READY,
        PLV_PLE_STATE_READY
    } PlvPipelineElementState;

    class PipelineElement : public QObject, public RefCounted
    {
        /** inherits from QObject thus Q_OBJECT macro is necessary */
        Q_OBJECT

        /** typedefs to make code more readable */
        typedef std::map< QString, RefPtr< IInputPin > > InputPinMap;
        typedef std::map< QString, RefPtr< IOutputPin > > OutputPinMap;

    protected:
        /** map which contains the input pins identified and indexed by their name */
        InputPinMap  m_inputPins;

        /** map which contains the output pins identified and indexed by their name */
        OutputPinMap m_outputPins;

    public:
        friend class Pipeline;

        /** QMetaType requires a public default constructor,
         *  a public copy constructor and a public destructor.
         */
        PipelineElement();
        PipelineElement( const PipelineElement& other );
        virtual ~PipelineElement();

        /** Initialise the element so it is ready to receive
          * process() calls.
          * Should this be reentrant?
          */
        virtual PlvPipelineElementState init() = 0;

        /** Adds the input pin to this processing element.
          * @throws IllegalArgumentException if an input pin with
          * the same name already exists
          */
        void addInputPin( IInputPin* pin ) throw (IllegalArgumentException);

        /** Adds the output pin to this processing element.
          * @throws IllegalArgumentException if an input pin with
          * the same name already exists
          */
        void addOutputPin( IOutputPin* pin ) throw (IllegalArgumentException);

        /** @returns the input pin with that name, or null if none exists */
        IInputPin* getInputPin( const QString& name ) const;

        /** @returns the ouput pin with that name, or null if none exists */
        IOutputPin* getOutputPin( const QString& name ) const;

        /** @returns a list of names of input pins added to this PipelineElement */
        std::list<QString>* getInputPinNames() const;

        /** @returns a list of names of output pins added to this PipelineElement */
        std::list<QString>* getOutputPinNames() const;

        /** @returns true when this PipelineElement is ready for procesing, which
          * means that the process method is allowed to be called by the scheduler. This
          * method is necessary to support processors which do not require input to be
          * available on all defined pins and hence makes it relatively easy to support
          * asynchronous events using normal pipeline connections. Also, processors could
          * be implemented as state machines, using pipeline connections as change of
          * state signals. For instance, one could design a processor which does A when
          * the light is on, and B when the light is not on, where the light state is
          * connected by a normal processor connection.
          */
        virtual bool isReadyForProcessing() const;

        /** This function does the actual work of this PipelineElement and
          * is called by the PipelineScheduler when inputs of this processor
          * are ready i.e. when isReadyForProcessing returns true.
          */
        virtual void process() = 0;

        /** Get a list of all known PipelineElement Type names
        */
        static std::list<QString> types();

        /** Register the given type as a PipelineElement Type.
        * The type needs to be known to Qt's MetaType system,
        * so you will likely rarely call this yourself.
        * Use one of the plvRegisterPipelineElement macros instead.
        * @require typeName was not registered to PipelineElement before.
        * @require typeName is a type registered to the Qt MetaType system
        *     e.g. QMetaType::type(typeName) returns a valid ID
        */
        static int registerType(QString typeName);


    protected:
        RefPtr<Pipeline> m_parent;

        // list to keep track of registered types
        static std::list<QString> s_types;

        /**
         * This gets called by Pipeline when we are added to it.
         * Handles removing ourself from any previous pipeline we were part of
         * and sets m_parent to the new pipeline
         */
        virtual void setPipeline(Pipeline* parent);

        /**
          * private process function which handles scoping of input and output pins
          * and class the process() function of the super class.
          */
        void __process();
    };
}

template<typename PET>
int plvRegisterPipelineElement(const char* typeName)
{
    plv::PipelineElement::registerType(typeName);
    return qRegisterMetaType<PET>(typeName);
}


#endif // PIPELINEELEMENT_H
