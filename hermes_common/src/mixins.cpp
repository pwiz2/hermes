// This file is part of HermesCommon
//
// Hermes is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Hermes is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Hermes; if not, see <http://www.gnu.prg/licenses/>.
#include "mixins.h"
#include "common.h"
#include "matrix.h"
#include "util/memory_handling.h"
#ifdef WIN32
#include <Windows.h>
#endif

namespace Hermes
{
  namespace Mixins
  {
    void StateQueryable::check() const
    {
      if (!this->isOkay())
      {
        std::stringstream ss;
        ss << "The instance of " << this->getClassName() << " is not OK.";
        throw Hermes::Exceptions::Exception(ss.str().c_str());
      }
    }

    char* Loggable::staticLogFileName = nullptr;

    Loggable::Loggable(bool verbose_output, callbackFn verbose_callback, bool add_newline) :
      verbose_output(verbose_output),
      verbose_callback(verbose_callback),
      logFileName(NULL),
      print_timestamps(true),
      erase_on_beginning(false),
      file_output_only(false),
      log_file_written(false),
      add_newline(add_newline)
    {
    }

    void Loggable::set_logFile_name(const char* filename)
    {
      free_with_check(this->logFileName);
      int strlength = std::strlen(filename);
      this->logFileName = malloc_with_check<char>(strlength + 1);
      strcpy(this->logFileName, filename);
    }

    void Loggable::set_logFile_name(std::string filename)
    {
      this->set_logFile_name(filename.c_str());
    }

    void Loggable::set_static_logFile_name(const char* filename)
    {
      free_with_check(Loggable::staticLogFileName);
      int strlength = std::strlen(filename);
      Loggable::staticLogFileName = malloc_with_check<char>(strlength + 1);
      strcpy(Loggable::staticLogFileName, filename);
    }

    void Loggable::set_static_logFile_name(std::string filename)
    {
      Loggable::set_static_logFile_name(filename.c_str());
    }

    void Loggable::set_file_output_only(bool onOff)
    {
      this->file_output_only = onOff;
    }

    bool Loggable::get_verbose_output() const
    {
      return this->verbose_output;
    }

    Loggable::callbackFn Loggable::get_verbose_callback() const
    {
      return this->verbose_callback;
    }

    void Loggable::Static::info(const char* msg, ...)
    {
      char text[BUF_SZ];
      char* text_contents = text + 1;

      text[0] = HERMES_EC_INFO;
      text[1] = ' ';
      text_contents++;

      //print the message
      va_list arglist;
      va_start(arglist, msg);
      vsprintf(text_contents, msg, arglist);
      va_end(arglist);

      //Windows platform
#ifdef _WINDOWS
      HANDLE h_console = GetStdHandle(STD_OUTPUT_HANDLE);

      //generate console settings
      WORD console_attr_red = FOREGROUND_RED, console_attr_green = FOREGROUND_GREEN, console_attr_blue = FOREGROUND_BLUE;

      WORD console_attrs = 0;
      console_attrs |= console_attr_green;

      //set new_ console settings
      SetConsoleTextAttribute(h_console, console_attrs);

      //write text
      DWORD num_written;
      BOOL write_success = WriteConsoleA(h_console, text_contents, strlen(text_contents), &num_written, nullptr);
      std::cout << std::endl;

      //Linux platform
#else
# define FOREGROUND_RED 1
# define FOREGROUND_GREEN 2
# define FOREGROUND_BLUE 4
      //console color code
      int console_attrs = 0;
      bool console_bold = true;

      printf("\033[%dm", console_attrs + 30);

      //emphasize and console bold
      if (console_bold)
        printf("\033[1m");

      //print text and reset settings
      printf("%s\033[0m\n", text_contents);

#endif
    }

    void Loggable::Static::warn(const char* msg, ...)
    {
      char text[BUF_SZ];
      char* text_contents = text + 1;

      text[0] = HERMES_EC_WARNING;
      text[1] = ' ';
      text_contents++;

      //print the message
      va_list arglist;
      va_start(arglist, msg);
      vsprintf(text_contents, msg, arglist);
      va_end(arglist);

      //Windows platform
#ifdef _WINDOWS
      HANDLE h_console = GetStdHandle(STD_OUTPUT_HANDLE);

      //generate console settings
      WORD console_attr_red = FOREGROUND_RED, console_attr_green = FOREGROUND_GREEN, console_attr_blue = FOREGROUND_BLUE;

      WORD console_attrs = 0;
      console_attrs |= console_attr_red | console_attr_green;

      //set new_ console settings
      SetConsoleTextAttribute(h_console, console_attrs);

      //write text
      DWORD num_written;
      BOOL write_success = WriteConsoleA(h_console, text_contents, strlen(text_contents), &num_written, nullptr);
      std::cout << std::endl;
      //Linux platform
#else
# define FOREGROUND_RED 1
# define FOREGROUND_GREEN 2
# define FOREGROUND_BLUE 4
      //console color code
      int console_attrs = 1;
      console_attrs |= FOREGROUND_RED | FOREGROUND_GREEN;
      bool console_bold = false;

      printf("\033[%dm", console_attrs + 30);

      //emphasize and console bold
      if (console_bold)
        printf("\033[1m");

      //print text and reset settings
      printf("%s\033[0m\n", text_contents);

#endif
    }

    void Loggable::Static::error(const char* msg, ...)
    {
      char text[BUF_SZ];
      char* text_contents = text + 1;

      text[0] = HERMES_EC_ERROR;
      text[1] = ' ';
      text_contents++;

      //print the message
      va_list arglist;
      va_start(arglist, msg);
      vsprintf(text_contents, msg, arglist);
      va_end(arglist);

      //Windows platform
#ifdef _WINDOWS
      HANDLE h_console = GetStdHandle(STD_OUTPUT_HANDLE);

      //generate console settings
      WORD console_attr_red = FOREGROUND_RED, console_attr_green = FOREGROUND_GREEN, console_attr_blue = FOREGROUND_BLUE;

      WORD console_attrs = 0;
      console_attrs |= console_attr_red;

      //set new_ console settings
      SetConsoleTextAttribute(h_console, console_attrs);

      //write text
      DWORD num_written;
      BOOL write_success = WriteConsoleA(h_console, text_contents, strlen(text_contents), &num_written, nullptr);
      std::cout << std::endl;
      //Linux platform
#else
# define FOREGROUND_RED 1
# define FOREGROUND_GREEN 2
# define FOREGROUND_BLUE 4
      //console color code
      int console_attrs = 1;
      console_attrs |= FOREGROUND_RED;
      bool console_bold = false;

      printf("\033[%dm", console_attrs + 30);

      //emphasize and console bold
      if (console_bold)
        printf("\033[1m");

      //print text and reset settings
      printf("%s\033[0m\n", text_contents);

#endif
    }

    void Loggable::error(const char* msg, ...) const
    {
      if (!this->verbose_output)
        return;

      char text[BUF_SZ];
      char* text_contents = text + 1;

      text[0] = HERMES_EC_ERROR;
      text[1] = ' ';
      text_contents++;

      //print the message
      va_list arglist;
      va_start(arglist, msg);
      vsprintf(text_contents, msg, arglist);
      va_end(arglist);
      hermes_log_message(HERMES_EC_ERROR, text_contents);
    }

    void Loggable::error_if(bool cond, const char* msg, ...) const
    {
      if (!this->verbose_output)
        return;

      if (cond)
      {
        char text[BUF_SZ];
        char* text_contents = text + 1;

        text[0] = HERMES_EC_ERROR;
        text[1] = ' ';
        text_contents++;

        //print the message
        va_list arglist;
        va_start(arglist, msg);
        vsprintf(text_contents, msg, arglist);
        va_end(arglist);
        hermes_log_message(HERMES_EC_ERROR, text_contents);
      }
    }

    void Loggable::warn(const char* msg, ...) const
    {
      if (!this->verbose_output)
        return;

      char text[BUF_SZ];
      char* text_contents = text + 1;

      text[0] = HERMES_EC_WARNING;
      text[1] = ' ';
      text_contents++;

      //print the message
      va_list arglist;
      va_start(arglist, msg);
      vsprintf(text_contents, msg, arglist);
      va_end(arglist);
      hermes_log_message(HERMES_EC_WARNING, text_contents);
    }

    void Loggable::warn_if(bool cond, const char* msg, ...) const
    {
      if (!this->verbose_output)
        return;

      if (cond)
      {
        char text[BUF_SZ];
        char* text_contents = text + 1;

        text[0] = HERMES_EC_WARNING;
        text[1] = ' ';
        text_contents++;

        //print the message
        va_list arglist;
        va_start(arglist, msg);
        vsprintf(text_contents, msg, arglist);
        va_end(arglist);
        hermes_log_message(HERMES_EC_WARNING, text_contents);
      }
    }
    void Loggable::info(const char* msg, ...) const
    {
      if (!this->verbose_output)
        return;

      char text[BUF_SZ];
      char* text_contents = text + 1;

      text[0] = HERMES_EC_INFO;
      text[1] = ' ';
      text_contents++;

      //print the message
      va_list arglist;
      va_start(arglist, msg);
      vsprintf(text_contents, msg, arglist);
      va_end(arglist);
      hermes_log_message(HERMES_EC_INFO, text_contents);
    }
    void Loggable::info_if(bool cond, const char* msg, ...) const
    {
      if (!this->verbose_output)
        return;

      if (cond)
      {
        char text[BUF_SZ];
        char* text_contents = text + 1;

        text[0] = HERMES_EC_INFO;
        text[1] = ' ';
        text_contents++;

        //print the message
        va_list arglist;
        va_start(arglist, msg);
        vsprintf(text_contents, msg, arglist);
        va_end(arglist);
        hermes_log_message(HERMES_EC_INFO, text_contents);
      }
    }

    bool Loggable::write_console(const char code, const char* text) const
    {
      //Windows platform
#ifdef _WINDOWS
      HANDLE h_console = GetStdHandle(STD_OUTPUT_HANDLE);
      if (h_console == INVALID_HANDLE_VALUE)
        return false;

      //read current console settings
      CONSOLE_SCREEN_BUFFER_INFO console_info;
      if (!GetConsoleScreenBufferInfo(h_console, &console_info))
        return false;

      //generate console settings
      WORD console_attr_red = FOREGROUND_RED, console_attr_green = FOREGROUND_GREEN, console_attr_blue = FOREGROUND_BLUE;

      WORD console_attrs = 0;
      switch (code)
      {
      case HERMES_EC_ERROR: console_attrs |= console_attr_red; break;
      case HERMES_EC_WARNING: console_attrs |= console_attr_red | console_attr_green; break;
      case HERMES_EC_INFO:console_attrs |= console_attr_green;  break;
      default: throw Hermes::Exceptions::Exception("Unknown error code: '%c'", code);
      }

      //set new_ console settings
      SetConsoleTextAttribute(h_console, console_attrs);

      //write text
      DWORD num_written;
      BOOL write_success = WriteConsoleA(h_console, text, strlen(text), &num_written, nullptr);

      //return previous settings
      SetConsoleTextAttribute(h_console, console_info.wAttributes);

      if (write_success)
        return true;
      else
        return false;
      //Linux platform
#else
# define FOREGROUND_RED 1
# define FOREGROUND_GREEN 2
# define FOREGROUND_BLUE 4
      //console color code
      int console_attrs = 0;
      bool console_bold = false;
      switch (code)
      {
      case HERMES_EC_WARNING: console_attrs |= FOREGROUND_RED | FOREGROUND_GREEN; break;
      case HERMES_EC_INFO: console_bold = true; break;
      default: throw Hermes::Exceptions::Exception("Unknown error code: '%c'", code);
      }

      printf("\033[%dm", console_attrs + 30);

      //emphasize and console bold
      if (console_bold)
        printf("\033[1m");

      //print text and reset settings
      printf("%s\033[0m", text);

      return true;
#endif
    }

    Loggable::HermesLogEventInfo* Loggable::hermes_build_log_info(char event) const
    {
      return new Loggable::HermesLogEventInfo(event, __CURRENT_FUNCTION, __FILE__, __LINE__);
    }

    void Loggable::hermes_fwrite(const void* ptr, size_t size, size_t nitems, FILE* stream) const
    {
      if (fwrite(ptr, size, nitems, stream) != nitems || ferror(stream))
        throw Hermes::Exceptions::Exception("Error writing to file: %s", strerror(ferror(stream)));
    }

    void Loggable::hermes_fread(void* ptr, size_t size, size_t nitems, FILE* stream) const
    {
      size_t ret = fread(ptr, size, nitems, stream);
      if (ret < nitems)
        throw Hermes::Exceptions::Exception("Premature end of file.");
      else if (ferror(stream))
        throw Hermes::Exceptions::Exception("Error reading file: %s", strerror(ferror(stream)));
    }

    Loggable::HermesLogEventInfo::HermesLogEventInfo(const char code, const char* src_function, const char* src_file, const int src_line)
      : code(code), src_function(src_function), src_file(src_file), src_line(src_line)
    {}

    void Loggable::set_timestamps(bool onOff)
    {
      this->print_timestamps = onOff;
    }

    void Loggable::set_erase_on_beginning(bool onOff)
    {
      this->erase_on_beginning = onOff;
    }

    void Loggable::hermes_log_message(const char code, const char* msg) const
    {
#pragma omp critical (hermes_log_message)
      {
        //print the message
        if (!this->file_output_only)
        {
          if (!write_console(code, msg))
            //safe fallback
            printf("%s", msg);

          //write a new line
          printf("\n");
        }

        HermesLogEventInfo* info = this->hermes_build_log_info(code);

        //print to file
        char* log_file_name = (this->logFileName ? this->logFileName : Loggable::staticLogFileName);
        if (log_file_name)
        {
          FILE* file;
          if (this->erase_on_beginning && !this->log_file_written)
            file = fopen(log_file_name, "wt");
          else
            file = fopen(log_file_name, "at");
          if (file != NULL)
          {
            //check whether log file was already written
            if (!log_file_written)
            {
              //first write, write delimited to a file
              (const_cast<Loggable*>(this))->log_file_written = true;
              if (!this->erase_on_beginning)
              {
                fprintf(file, "\n");
                for (int i = 0; i < HERMES_LOG_FILE_DELIM_SIZE; i++)
                  fprintf(file, "-");
                fprintf(file, "\n\n");
              }
            }

            if (print_timestamps)
            {
              //get time
              time_t now;
              time(&now);
              struct tm* now_tm = gmtime(&now);
              char time_buf[BUF_SZ];
              strftime(time_buf, BUF_SZ, "%y%m%d-%H:%M", now_tm);
              //write
              fprintf(file, "%s\t%s", time_buf, msg);
            }
            else
              fprintf(file, "%s", msg);

            if (this->add_newline)
              fprintf(file, "\n");

            fclose(file);

            if (this->verbose_callback != nullptr)
              this->verbose_callback(msg);
          }
        }

        delete info;
      }
    }

    void Loggable::set_verbose_output(bool to_set)
    {
      this->verbose_output = to_set;
    }

    void Loggable::set_verbose_callback(callbackFn callback)
    {
      this->verbose_callback = callback;
    }

    TimeMeasurable::TimeMeasurable(const char *name) : period_name(name == nullptr ? "unnamed" : name)
    {
      //initialization
#ifdef _WINDOWS  //Windows
      LARGE_INTEGER freq;
      if (QueryPerformanceFrequency(&freq))
        frequency = (double)freq.QuadPart;
      else
        frequency = -1;
#endif  //Linux
      tick_reset();
    }

    TimeMeasurable::SysTime TimeMeasurable::get_time() const
    {
#ifdef _WINDOWS  //Windows
      if (frequency > 0)
      {
        LARGE_INTEGER ticks;
        QueryPerformanceCounter(&ticks);
        return ticks.QuadPart;
      }
      else
      {
        return clock();
      }
#elif defined(__APPLE__)  //Mac
      // FIXME: implement time measurement on Mac
      timespec tm;
      return tm;
#else  //Linux
      timespec tm;
      clock_gettime(CLOCK_REALTIME, &tm);
      return tm;
#endif
    }

    double TimeMeasurable::period_in_seconds(const SysTime& begin, const SysTime& end) const
    {
#ifdef _WINDOWS  //Windows
      uint64_t period = end - begin;
      if (frequency > 0)
        return period / frequency;
      else
        return period / (double)CLOCKS_PER_SEC;
#else  //Linux
      int sec_corr = 0;
      long period_nsec = end.tv_nsec - begin.tv_nsec;
      if (period_nsec < 0)
      {
        sec_corr += -1;
        period_nsec += 1000000000UL;
      }
      long period_sec = (long)(end.tv_sec - begin.tv_sec) + sec_corr;
      return period_sec + (period_nsec / 1E9);
#endif
    }

    const TimeMeasurable& TimeMeasurable::tick(TimerPeriodTickType type)
    {
      SysTime cur_time = get_time();
      if (type == HERMES_ACCUMULATE)
      {
        double secs = period_in_seconds(last_time, cur_time);
        accum += secs;
        last_period = secs;
      }
      else
        last_period = 0.0;

      last_time = cur_time;
      return *this;
    }

    const std::string& TimeMeasurable::name() const
    {
      return period_name;
    }

    double TimeMeasurable::accumulated() const
    {
      return accum;
    }

    std::string TimeMeasurable::accumulated_str() const {
      return to_string(accum);
    }

    double TimeMeasurable::last() const
    {
      return last_period;
    }

    std::string TimeMeasurable::last_str() const
    {
      return to_string(last_period);
    }

    const TimeMeasurable& TimeMeasurable::tick_reset()
    {
      tick(HERMES_SKIP);
      reset();
      return *this;
    }

    const TimeMeasurable& TimeMeasurable::reset()
    {
      accum = 0;
      last_time = get_time();
      last_period = 0.0;
      return *this;
    }

    std::string TimeMeasurable::to_string(double secs) const
    {
      if (secs < 0)
        return "NO TIME";
      else
      {
        int hours = (int)secs / (3600);
        int mins = (int)fmod(secs, 3600) / 60;
        secs = fmod(secs, 60);

        std::stringstream str;
        if (hours > 0)
          str << hours << "h ";
        if (hours > 0 || mins > 0)
          str << mins << "m ";
        str << secs << "s";

        return str.str();
      }
    }

    IntegrableWithGlobalOrder::IntegrableWithGlobalOrder() : global_integration_order_set(false), global_integration_order(0)
    {}

    void IntegrableWithGlobalOrder::set_global_integration_order(unsigned int order)
    {
      this->global_integration_order = order;
      this->global_integration_order_set = true;
    }

    SettableComputationTime::SettableComputationTime() : time(0.0), time_step(0.0)
    {
    }

    void SettableComputationTime::set_time(double time)
    {
      this->time = time;
    }

    void SettableComputationTime::set_time_step(double time_step)
    {
      this->time_step = time_step;
    }

    OutputAttachable::OutputAttachable()
    {
    }

    bool OutputAttachable::on_initialization()
    {
      return true;
    }

    bool OutputAttachable::on_step_begin()
    {
      return true;
    }

    bool OutputAttachable::on_initial_step_end()
    {
      return true;
    }

    bool OutputAttachable::on_step_end()
    {
      return true;
    }

    bool OutputAttachable::on_finish()
    {
      return true;
    }

    template<typename T>
    const T& OutputAttachable::get_parameter_value(const Parameter<T>& parameter)
    {
      return *parameter.value;
    }

    template<typename T>
    T& OutputAttachable::get_parameter_value(Parameter<T>& parameter)
    {
      return *parameter.value;
    }

    template<typename T>
    void OutputAttachable::set_parameter_value(Parameter<T>& parameter, T* value)
    {
      parameter.value = value;
    }

    template HERMES_API const unsigned int& OutputAttachable::get_parameter_value<unsigned int>(const Parameter<unsigned int>& parameter);
    template HERMES_API const double& OutputAttachable::get_parameter_value<double>(const Parameter<double>& parameter);
    template HERMES_API const bool& OutputAttachable::get_parameter_value<bool>(const Parameter<bool>& parameter);
    template HERMES_API const std::vector<unsigned int>& OutputAttachable::get_parameter_value<std::vector<unsigned int> >(const Parameter<std::vector<unsigned int> >& parameter);
    template HERMES_API const std::vector<double>& OutputAttachable::get_parameter_value<std::vector<double> >(const Parameter<std::vector<double> >& parameter);
    template HERMES_API const std::vector<bool>& OutputAttachable::get_parameter_value<std::vector<bool> >(const Parameter<std::vector<bool> >& parameter);

    template HERMES_API unsigned int& OutputAttachable::get_parameter_value<unsigned int>(Parameter<unsigned int>& parameter);
    template HERMES_API double& OutputAttachable::get_parameter_value<double>(Parameter<double>& parameter);
    template HERMES_API bool& OutputAttachable::get_parameter_value<bool>(Parameter<bool>& parameter);
    template HERMES_API std::vector<unsigned int>& OutputAttachable::get_parameter_value<std::vector<unsigned int> >(Parameter<std::vector<unsigned int> >& parameter);
    template HERMES_API std::vector<double>& OutputAttachable::get_parameter_value<std::vector<double> >(Parameter<std::vector<double> >& parameter);
    template HERMES_API std::vector<bool>& OutputAttachable::get_parameter_value<std::vector<bool> >(Parameter<std::vector<bool> >& parameter);

    template HERMES_API void OutputAttachable::set_parameter_value<unsigned int>(Parameter<unsigned int>& parameter, unsigned int* value);
    template HERMES_API void OutputAttachable::set_parameter_value<double>(Parameter<double>& parameter, double* value);
    template HERMES_API void OutputAttachable::set_parameter_value<bool>(Parameter<bool>& parameter, bool* value);
    template HERMES_API void OutputAttachable::set_parameter_value<std::vector<unsigned int> >(Parameter<std::vector<unsigned int> >& parameter, std::vector<unsigned int>* value);
    template HERMES_API void OutputAttachable::set_parameter_value<std::vector<double> >(Parameter<std::vector<double> >& parameter, std::vector<double>* value);
    template HERMES_API void OutputAttachable::set_parameter_value<std::vector<bool> >(Parameter<std::vector<bool> >& parameter, std::vector<bool>* value);
  }
}