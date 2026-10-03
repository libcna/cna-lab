package Microsoft.Xna.Framework;

import java.util.AbstractList;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.concurrent.CopyOnWriteArrayList;

/** Ordered mutable component collection with XNA add/remove event semantics. */
@SuppressWarnings("serial")
public final class GameComponentCollection extends AbstractList<IGameComponent> {

    private final List<IGameComponent> items = new ArrayList<>();
    private final CopyOnWriteArrayList<EventHandler<GameComponentCollectionEventArgs>> addedListeners =
            new CopyOnWriteArrayList<>();
    private final CopyOnWriteArrayList<EventHandler<GameComponentCollectionEventArgs>> removedListeners =
            new CopyOnWriteArrayList<>();

    public GameComponentCollection() {
    }

    public final void addComponentAddedListener(
            EventHandler<GameComponentCollectionEventArgs> listener) {
        addedListeners.add(Objects.requireNonNull(listener, "listener"));
    }

    public final void removeComponentAddedListener(
            EventHandler<GameComponentCollectionEventArgs> listener) {
        addedListeners.remove(listener);
    }

    public final void addComponentRemovedListener(
            EventHandler<GameComponentCollectionEventArgs> listener) {
        removedListeners.add(Objects.requireNonNull(listener, "listener"));
    }

    public final void removeComponentRemovedListener(
            EventHandler<GameComponentCollectionEventArgs> listener) {
        removedListeners.remove(listener);
    }

    @Override
    public IGameComponent get(int index) {
        return items.get(index);
    }

    @Override
    public int size() {
        return items.size();
    }

    @Override
    public void add(int index, IGameComponent element) {
        InsertItem(index, element);
    }

    @Override
    public IGameComponent set(int index, IGameComponent element) {
        IGameComponent previous = items.get(index);
        SetItem(index, element);
        return previous;
    }

    @Override
    public IGameComponent remove(int index) {
        IGameComponent previous = items.get(index);
        RemoveItem(index);
        return previous;
    }

    @Override
    public void clear() {
        ClearItems();
    }

    protected void ClearItems() {
        List<IGameComponent> removed = List.copyOf(items);
        items.clear();
        modCount++;
        for (IGameComponent item : removed) {
            fire(removedListeners, item);
        }
    }

    protected void InsertItem(int index, IGameComponent item) {
        IGameComponent value = Objects.requireNonNull(item, "item");
        items.add(index, value);
        modCount++;
        fire(addedListeners, value);
    }

    protected void RemoveItem(int index) {
        IGameComponent removed = items.remove(index);
        modCount++;
        fire(removedListeners, removed);
    }

    protected void SetItem(int index, IGameComponent item) {
        IGameComponent value = Objects.requireNonNull(item, "item");
        IGameComponent removed = items.set(index, value);
        modCount++;
        fire(removedListeners, removed);
        fire(addedListeners, value);
    }

    private void fire(
            CopyOnWriteArrayList<EventHandler<GameComponentCollectionEventArgs>> listeners,
            IGameComponent component) {
        GameComponentCollectionEventArgs args = new GameComponentCollectionEventArgs(component);
        for (EventHandler<GameComponentCollectionEventArgs> listener : listeners) {
            listener.invoke(this, args);
        }
    }
}
