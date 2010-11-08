#ifndef IAPPLICATION_H
#define IAPPLICATION_H

#include <string>

namespace tsdc {

class IApplication
{
	public:
		virtual ~IApplication() {}
		virtual bool IsShutdown() const = 0;
		virtual void Start() = 0;
		virtual void Shutdown() = 0;
		virtual void LoadScene(const std::string &file) = 0;
		virtual void UnloadScene() = 0;
		virtual void CaptureInput() = 0;

	protected:
		virtual void Run() = 0;
}; // class IApplication

class BaseApplication : public IApplication
{
	public:
		virtual bool IsShutdown() const { return this->_is_shutdown; }

	protected:
		bool _is_shutdown;
}; // class BaseApplication

}; // namespace tsdc

#endif // IAPPLICATION_H