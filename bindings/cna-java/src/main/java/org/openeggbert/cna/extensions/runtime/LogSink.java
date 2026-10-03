package org.openeggbert.cna.extensions.runtime;

/**
 * Receives one log line CNA wrote.
 *
 * <p>A CNA extension. CNA's own default sink writes to <strong>stderr, deliberately never
 * stdout</strong>, because a terminal-hosted game draws its frame on stdout and a log line there
 * would corrupt it. That is why a replaceable sink exists at all, and it is why a Java one is
 * worth having: a game that already logs somewhere -- a file, a logging framework, an in-game
 * console -- can put CNA's own lines in the same place rather than beside them.
 *
 * <p><strong>What a sink must not do.</strong> CNA calls it from whichever thread wrote the line,
 * and states two rules: a sink must not call back into CNA, and it must return normally. The
 * second one this projection can soften and does -- an exception thrown here is described on the
 * standard error stream and swallowed, because a sink that throws while reporting a log line must
 * not also take the process down -- but the first it cannot. Logging from inside a sink, or
 * touching any CNA object from one, is undefined.
 *
 * @see CnaLogger#setSink(LogSink)
 */
@FunctionalInterface
public interface LogSink {

    /**
     * Handles one formatted line.
     *
     * @param level the line's level
     * @param category what part of the runtime wrote it
     * @param message the formatted line, without a trailing newline
     */
    void accept(LogLevel level, LogCategory category, String message);
}
