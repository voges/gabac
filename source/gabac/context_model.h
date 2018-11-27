#ifndef GABAC_CONTEXT_MODEL_H_
#define GABAC_CONTEXT_MODEL_H_


namespace gabac {


class ContextModel
{
 public:
    explicit ContextModel(
            unsigned char state
    );

    ~ContextModel();

    unsigned char getState() const;

    unsigned char getMps() const;

    void updateLps();

    void updateMps();

 private:
    unsigned char m_state;
};


}  // namespace gabac


#endif  // GABAC_CONTEXT_MODEL_H_
