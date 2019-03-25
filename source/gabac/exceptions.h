#ifndef GABAC_EXCEPTIONS_H_
#define GABAC_EXCEPTIONS_H_

#define GABAC_DIE(message) \
    throw gabac::RuntimeException(__FILE__, __FUNCTION__, __LINE__, message)


#include <exception>
#include <string>


namespace gabac {

/**
 * General exception
 */
class Exception : public std::exception
{
 public:
    /**
     * Create a new exception
     * @param message Reason of error
     */
    explicit Exception(
            const std::string& message
    );

    /**
     * Destroy exception
     */
    ~Exception() noexcept override;

    /**
     * Return the exception message
     * @return Reason of error
     */
    virtual std::string message() const;

    /**
     * Return the exception message as char*
     * @return Reason of error
     */
    const char *what() const noexcept override;

 protected:
    std::string m_message;
};

/**
 * Exception specialized for runtime information
 */
class RuntimeException : public Exception
{
 public:
    /**
     * Create new runtime exception
     * @param file File the error occurred
     * @param function Function the error occurred
     * @param line Line the error occurred
     * @param message Reason of error
     */
    explicit RuntimeException(
            const std::string& file,
            const std::string& function,
            int line,
            const std::string& message
    ) noexcept;

    /**
     * Copy construction
     * @param e Source
     */
    RuntimeException(
            const RuntimeException& e
    ) noexcept;

    /**
     * Destroy exception
     */
    ~RuntimeException() noexcept override;
};


}  // namespace gabac


#endif  // GABAC_EXCEPTIONS_H_
