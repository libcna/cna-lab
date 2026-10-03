package org.openeggbert.cna.extensions.input;

import java.util.List;

/**
 * The list of words an input method is offering for the current composition.
 *
 * <p>A CNA extension. A game that draws its own text field has to draw this list itself, because
 * the host will not: it reports the candidates and leaves the presentation to the application.
 *
 * @param Candidates the words on offer, in the host's order, possibly empty
 * @param SelectedIndex the index the host has pre-selected, or -1 for none. CNA passes this
 *     through without range-checking it, so it is not guaranteed to be a valid index;
 *     {@link #getSelected()} is the safe way to read it.
 * @param Horizontal whether the host lays the list out horizontally rather than vertically
 */
public record TextCandidates(List<String> Candidates, int SelectedIndex, boolean Horizontal) {

    /** Copies the candidate list, so an event cannot be changed after it is delivered. */
    public TextCandidates {
        Candidates = List.copyOf(Candidates);
    }

    /**
     * Returns the pre-selected candidate.
     *
     * @return the word, or {@code null} when nothing is selected or the host's index is not one
     *     of the candidates it sent
     */
    public String getSelected() {
        if (SelectedIndex < 0 || SelectedIndex >= Candidates.size()) {
            return null;
        }
        return Candidates.get(SelectedIndex);
    }
}
